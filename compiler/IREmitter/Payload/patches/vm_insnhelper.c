
void sorbet_setExceptionStackFrame(rb_execution_context_t *ec, rb_control_frame_t *cfp, const rb_iseq_t *iseq) {
    // Self is the same in exception-handlers
    VALUE self = cfp->self;

    // there is only ever one local for rescue/ensure frames, this mirrors the implementation in
    // https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/vm.c#L2085-L2101
    VALUE *sp = cfp->sp + 1;
    int num_locals = iseq->body->local_table_size - 1;

    VALUE blockHandler = VM_GUARDED_PREV_EP(cfp->ep);
    VALUE me = 0;

    // write the exception value on the stack (as an argument to the rescue frame)
    cfp->sp[0] = rb_errinfo();

    // NOTE: there's no explicit check for stack overflow, because `vm_push_frame` will do that check
    cfp = vm_push_frame(ec, iseq, VM_FRAME_MAGIC_RESCUE, self, blockHandler, me, iseq->body->iseq_encoded, sp,
                        num_locals, iseq->body->stack_max);
}

void sorbet_popRubyStack() {
    rb_execution_context_t *ec = GET_EC();
    vm_pop_frame(ec, ec->cfp, ec->cfp->ep);
}

// NOTE: this is marked noinline so that there's only ever one copy that lives in the ruby runtime, cutting down on the
// size of generated artifacts. If we decide that speed is more important, this could be marked alwaysinline to avoid
// the function call.
void sorbet_setMethodStackFrame(rb_execution_context_t *ec, rb_control_frame_t *cfp, const rb_iseq_t *iseq) {
    // make sure that we have enough space to allocate locals
    int local_size = iseq->body->local_table_size;
    int stack_max = iseq->body->stack_max;

    CHECK_VM_STACK_OVERFLOW0(cfp, cfp->sp, local_size + stack_max);

    // save the current state of the stack
    VALUE cref_or_me = cfp->ep[VM_ENV_DATA_INDEX_ME_CREF]; // -2
    VALUE prev_ep = cfp->ep[VM_ENV_DATA_INDEX_SPECVAL];    // -1
    VALUE type = cfp->ep[VM_ENV_DATA_INDEX_FLAGS];         // -0

    // C frames push no locals, so the address we want to treat as the top of the stack lies at -2.
    // NOTE: we're explicitly discarding the const qualifier from ep, because we need to write to the stack.
    VALUE *sp = (VALUE *)&cfp->ep[-2];

    // setup locals
    for (int i = 0; i < local_size; ++i) {
        *sp++ = RUBY_Qnil;
    }

    // restore the previous state of the stack
    *sp++ = cref_or_me;
    *sp++ = prev_ep;
    *sp = type;

    cfp->ep = sp;
    cfp->__bp__ = sp;
    cfp->sp = sp + 1;
}

// compiler is closely aware of layout of this struct
struct FunctionInlineCache {
    // We use an `rb_kwarg_call_data` instead of `rb_call_data` as they contain the same data, and the kwarg variant
    // only adds a single pointer's worth of additional space.
    struct rb_kwarg_call_data cd;
};

// Initialize a method send cache. The values passed in for the keys must all be symbol values, and argc includes
// num_kwargs.
void sorbet_setupFunctionInlineCache(struct FunctionInlineCache *cache, ID mid, unsigned int flags, int argc,
                                     int num_kwargs, VALUE *keys) {
    struct rb_kwarg_call_data *cd = &cache->cd;

    cd->ci_kw.ci.mid = mid;
    cd->ci_kw.ci.orig_argc = argc;

    // TODO(trevor) will we need non-FCALL sends?
    cd->ci_kw.ci.flag = VM_CALL_FCALL | flags;

    if (num_kwargs > 0) {
        // The layout for struct_rb_call_info_with_kwarg has a 1-element array as the last field, so allocating
        // additional space will extend that array's length.
        struct rb_call_info_kw_arg *kw_arg = (struct rb_call_info_kw_arg *)rb_xmalloc_mul_add(
            num_kwargs - 1, sizeof(VALUE), sizeof(struct rb_call_info_kw_arg));

        kw_arg->keyword_len = num_kwargs;
        memcpy(&kw_arg->keywords, keys, num_kwargs * sizeof(VALUE));

        cd->ci_kw.kw_arg = kw_arg;
    } else {
        cd->ci_kw.kw_arg = NULL;
    }
}

// This send primitive assumes that all argumenst have been pushed to the ruby stack, and will invoke the vm machinery
// to execute the send.
VALUE sorbet_callFuncWithCache(struct FunctionInlineCache *cache, VALUE bh) {
    rb_execution_context_t *ec = GET_EC();
    rb_control_frame_t *cfp = ec->cfp;

    VALUE val = vm_sendish(ec, cfp, (struct rb_call_data *)&cache->cd, bh, vm_search_method_wrap);
    if (val == Qundef) {
        VM_ENV_FLAGS_SET(ec->cfp->ep, VM_FRAME_FLAG_FINISH);

        // false here because we don't want to consider jit frames
        val = rb_vm_exec(ec, false);
    }

    return val;
}

// This should only be called from a context where we know that a block handler has been passed.
VALUE sorbet_getPassedBlockHandler() {
    // this is an inlined version of PASS_PASSED_BLOCK_HANDLER()
    rb_execution_context_t *ec = GET_EC();
    const VALUE *ep = ec->cfp->ep;
    while (!VM_ENV_LOCAL_P(ep)) {
        ep = (void *)(ep[VM_ENV_DATA_INDEX_SPECVAL] & ~0x03);
    }
    VALUE block_handler = VM_ENV_BLOCK_HANDLER(ep);
    vm_block_handler_verify(block_handler);
    return block_handler;
}

void sorbet_vm_env_write_slowpath(const VALUE *ep, int index, VALUE v) {
    return vm_env_write_slowpath(ep, index, v);
}

VALUE sorbet_vm_splatIntrinsic(VALUE thing) {
    const VALUE dont_duplicate_input_arrays = Qfalse;
    return vm_splat_array(dont_duplicate_input_arrays, thing);
}
