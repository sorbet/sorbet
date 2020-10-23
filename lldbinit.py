if __name__ == "__main__":
    print("Run only as script from LLDB... Not as standalone program!")

try:
    import lldb
except:
    pass

import re
import os

def __lldb_init_module(debugger, internal_dict):
    result = lldb.SBCommandReturnObject()
    init_source_map(debugger, result)
    ci = debugger.GetCommandInterpreter()
    ci.HandleCommand("command script add -f lldbinit.cmd_dump_sorbet dumpsorbet", result)
    register_summary(ci, format_sorbet_core_Name.__name__, "sorbet::core::Name")
    register_summary(ci, format_sorbet_core_NameRef.__name__, "sorbet::core::NameRef")
    register_summary(ci, format_sorbet_core_GlobalState.__name__, "sorbet::core::GlobalState")
    # LLDB will not automatically walk up inheritance chains to find a formatter so it needs to
    # be specified for every subclass of Instruction
    register_summary(ci, format_sorbet_object.__name__, "sorbet::cfg::Instruction")
    register_summary(ci, format_sorbet_object.__name__, "sorbet::cfg::Ident")
    register_summary(ci, format_sorbet_object.__name__, "sorbet::cfg::Send")
    register_summary(ci, format_sorbet_object.__name__, "sorbet::cfg::CFG")
    register_summary(ci, format_sorbet_object.__name__, "sorbet::cfg::VariableUseSite")
    register_summary(ci, format_sorbet_object.__name__, "sorbet::cfg::LocalRef")

def register_summary(ci, funcName, typeName):
    result = lldb.SBCommandReturnObject()
    ci.HandleCommand("type summary add -F %s.%s %s" % (__name__, funcName, typeName), result)
    return result

def init_source_map(debugger, result):
    # Bazel builds all binaries from a username-specific cache location
    # for lldb to work properly we need to map that path back to our main source
    project_root = os.path.dirname(os.path.realpath(__file__))
    bazel_workspace = os.path.realpath(os.path.join(project_root, "bazel-sorbet"))

    if not(os.path.isdir(bazel_workspace)):
        result.Print("You need to compile Sorbet first before loading this file")
    else:
        result.Print("(lldb) Mapping paths in ./bazel-sorbet to the project root")
        source_map_command = 'settings set -- target.source-map "%s" "%s"' % (bazel_workspace, project_root)
        ci = debugger.GetCommandInterpreter()
        res = lldb.SBCommandReturnObject()
        ci.HandleCommand(source_map_command, res)

def get_variable_in_frames(frame, typeNames):
    if not(isinstance(typeNames, list)):
        typeNames = [typeNames]
    original_frame = frame
    while not(frame is None):
        all_vars = frame.get_all_variables()
        result = next((v for v in all_vars if any(typeName in v.GetTypeName() for typeName in typeNames)), None)
        if not(result is None):
            resultType = result.GetType()
            if result.TypeIsPointerType():
                result = result.Dereference()
            ## Parameters passed by reference are a bit special because they can't be dereferenced like a pointer would
            # be, but since they are really just pointers in disguise we can manually retrieve the underlying address
            # they represent and reconstruct a value of the right type from it
            if resultType.IsReferenceType():
                error = lldb.SBError()
                ref_data = result.GetData()
                ref_addr = ref_data.GetAddress(error, 0)
                if error.Success():
                    result = result.CreateValueFromAddress("__deref" + result.get_expr_path(), ref_addr, resultType.GetDereferencedType())
            if frame != original_frame:
                result = result.Persist()
            return result
        frame = frame.get_parent_frame()
    return None

def format_argument(valobj):
    exprPath = valobj.get_expr_path()
    # handle values that are smart pointer and need an extra manual indirection
    typeName = valobj.GetTypeName()
    if "::unique_ptr" in typeName or "::shared_ptr" in typeName:
        return '(*(' + exprPath + '))'
    return exprPath

def sorbet_obj_toString(sorbetObj, error = None):
    if sorbetObj.TypeIsPointerType():
        sorbetObj = sorbetObj.Dereference()
    sorbetType = sorbetObj.GetType()
    frame = sorbetObj.GetFrame()
    exprMethods = [sorbetType.GetMemberFunctionAtIndex(i) for i in range(sorbetType.GetNumberOfMemberFunctions())]
    toStringMethod = next((m for m in exprMethods if m.GetName() == "toString"), None)
    if toStringMethod is None:
        if not(error is None):
            error.SetErrorString("Object of type %s doesn't have a toString method" % sorbetType)
        return None
    # Try to retrieve GS and CFG from the current context
    gs = get_variable_in_frames(frame, ["sorbet::core::GlobalState", "sorbet::core::Context"])
    if gs is None:
        if not(error is None):
            error.SetErrorString("Couldn't find a GS instance in the current frame")
        return None
    cfg = None
    if toStringMethod.GetNumberOfArguments() == 2:
        cfg = get_variable_in_frames(frame, "sorbet::cfg::CFG")
        if cfg is None:
            if not(error is None):
                error.SetErrorString("Couldn't find a CFG instance in the current frame")
            return None
    globalStateAccess = "%s%s" % (format_argument(gs), ".state" if "sorbet::core::Context" in gs.GetTypeName() else "")
    command = sorbetObj.get_expr_path()
    cmd = "(%s).toString(%s)" % (command, globalStateAccess)
    if toStringMethod.GetNumberOfArguments() == 2:
        cmd = "(%s).toString(%s, %s)" % (command, globalStateAccess, format_argument(cfg))
    finalExpr = frame.EvaluateExpression(cmd)
    if not(finalExpr.IsValid()):
        if not(error is None):
            error.SetErrorString("No result was returned from expression evaluation")
        return None
    return finalExpr

def cmd_dump_sorbet(debugger, command, exe_ctx, result, internal_dict):
    error = lldb.SBError()
    error.SetErrorToGenericError()
    frame = exe_ctx.GetFrame()
    # Value that we are supposed to dump and retrieve its type to find a toString method
    expr = frame.EvaluateExpression(command)
    finalExpr = sorbet_obj_toString(expr, error)
    if not(finalExpr is None):
        result.Print("R: %s\n" % finalExpr)
    else:
        result.Print("no result\n")
        result.SetError(error)

def format_sorbet_core_NameRef(valobj, internal_dict, options):
    id = valobj.GetChildMemberWithName("_id").GetValueAsSigned(-1)
    if id <= 0:
        return "Default ID"
    elif id < 452:
        # Keep bound check in sync with NAME_LAST_WELL_KNOWN_NAME from autogenerated file Names_gen.h
        name = valobj.EvaluateExpression("::sorbet::core::Names::DEBUG_NAMES_LOOKUP[%d]" % id)
        name.SetFormat(lldb.eFormatCString)
        return name.GetValue()
    else:
        # Try to get the toString version of it
        stringified = sorbet_obj_toString(valobj)
        return ("Dynamic ID %d" % id) if stringified is None else stringified.GetSummary()

def format_sorbet_core_Name(valobj, internal_dict, options):
    kind = valobj.GetChildMemberWithName("kind")
    if kind.GetValueAsSigned() == 1:
        return valobj.GetChildMemberWithName("utf8").GetSummary()
    else:
        return "Name kind=%s" % kind.GetSummary()

def format_sorbet_core_GlobalState(valobj, internal_dict, options):
    if valobj.TypeIsPointerType():
        valobj = valobj.Dereference()
    expr = "%s.toString().c_str()" % valobj.get_expr_path()
    result = valobj.EvaluateExpression(expr)
    data = result.GetData()
    error = lldb.SBError()
    str_ptr = data.GetAddress(error, 0)
    str = valobj.GetProcess().ReadCStringFromMemory(str_ptr, 4096, error)
    if len(str) >= 4095:
        str = str + '...'
    return str if error.Success () and not(result is None) else '<error:' + error.GetCString() + '>'

def format_sorbet_object(valobj, internal_dict, options):
    error = lldb.SBError()
    result = sorbet_obj_toString(valobj)
    if result is None:
        if not(error.Success()):
            return '<error:' + error.GetCString() + '>'
        return valobj.GetValue()
    else:
        return result.GetSummary()
