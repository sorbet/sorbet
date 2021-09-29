
// compiler is closely aware of layout of this struct
struct FunctionInlineCache {
    // We use an `rb_kwarg_call_data` instead of `rb_call_data` as they contain the same data, and the kwarg variant
    // only adds a single pointer's worth of additional space.
    struct rb_kwarg_call_data cd;
};

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

    // This mirrors what the Ruby VM does for rescue frames:
    // https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/vm.c#L2094-L2105
    ec->tag->state = TAG_NONE;
    ec->errinfo = Qnil;
}

rb_control_frame_t *sorbet_pushStaticInitFrame(VALUE recv) {
    rb_execution_context_t *ec = GET_EC();

    VALUE cref = (VALUE)vm_cref_push(ec, recv, NULL, FALSE); // Qnil or T_IMEMO(cref) or T_IMEMO(ment)
    VALUE frame_type = VM_FRAME_MAGIC_CLASS | VM_ENV_FLAG_LOCAL;

    // TODO(trevor) we could pass this in to supply a block
    VALUE block_handler = VM_BLOCK_HANDLER_NONE;

    return vm_push_frame(ec, NULL, frame_type, recv, block_handler, cref, 0, ec->cfp->sp, 0, 0);
}

rb_control_frame_t *sorbet_pushCfuncFrame(struct FunctionInlineCache *cache, VALUE recv, const rb_iseq_t *iseq) {
    rb_execution_context_t *ec = GET_EC();

    // NOTE: method search must be done to ensure that this field is not NULL
    const rb_callable_method_entry_t *me = cache->cd.cc.me;
    VALUE frame_type = VM_FRAME_MAGIC_CFUNC | VM_ENV_FLAG_LOCAL;

    // TODO(trevor) we could pass this in to supply a block
    VALUE block_handler = VM_BLOCK_HANDLER_NONE;

    /* cf. vm_call_sorbet_with_frame_normal */
    return vm_push_frame(ec, iseq, frame_type, recv, block_handler, (VALUE)me, 0, ec->cfp->sp,
                         iseq->body->local_table_size, iseq->body->stack_max);
}

void sorbet_pushBlockFrame(const struct rb_captured_block *captured) {
    rb_execution_context_t *ec = GET_EC();
    vm_push_frame(ec, (const rb_iseq_t *)captured->code.ifunc, VM_FRAME_MAGIC_IFUNC | VM_FRAME_FLAG_CFRAME,
                  captured->self, VM_GUARDED_PREV_EP(captured->ep), (VALUE)NULL, 0, ec->cfp->sp, 0, 0);
}

void sorbet_popFrame() {
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

// Initialize a method send cache. The values passed in for the keys must all be symbol values, and argc includes
// num_kwargs.
void sorbet_setupFunctionInlineCache(struct FunctionInlineCache *cache, ID mid, unsigned int flags, int argc,
                                     int num_kwargs, VALUE *keys) {
    struct rb_kwarg_call_data *cd = &cache->cd;

    cd->ci_kw.ci.mid = mid;
    cd->ci_kw.ci.orig_argc = argc;
    cd->ci_kw.ci.flag = flags;

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

void sorbet_vmMethodSearch(struct FunctionInlineCache *cache, VALUE recv) {
    // we keep an `rb_kwarg_call_data` in FunctionInline cache.
    struct rb_call_data *cd = (struct rb_call_data *)&cache->cd;
    vm_search_method(cd, recv);
}

// Send Support ********************************************************************************************************

// Wrapped version of vm_call_opt_call:
// https://github.com/ruby/ruby/blob/5445e0435260b449decf2ac16f9d09bae3cafe72/vm_insnhelper.c#L2683-L2691
// The wrapper catches TAG_RETURN jumps (from return statements inside lambdas), and handles appropriately.
static VALUE sorbet_vm_call_opt_call(rb_execution_context_t *ec, rb_control_frame_t *reg_cfp,
                                     struct rb_calling_info *calling, struct rb_call_data *cd) {
    enum ruby_tag_type state;
    VALUE retval;
    rb_control_frame_t *const volatile save_cfp = ec->cfp;

    // Push a tag to the execution context's stack, so that subsequent longjmps (inside EC_JUMP_TAG) will know where to
    // jump to.
    EC_PUSH_TAG(ec);

    // If EC_EXEC_TAG() returns 0, the tag has just been set (including setjmp), and we continue with a call to
    // vm_call_opt_call.
    if ((state = EC_EXEC_TAG()) == 0) {
        retval = vm_call_opt_call(ec, reg_cfp, calling, cd);
    }
    // If EC_EXEC_TAG() returns TAG_RETURN, we have caught a longjmp (via EC_JUMP_TAG) from someone below us in the
    // call stack indicating they would like to do a non-local 'return'. We need to examine the vm_throw_data to decide
    // whether the return should go to the current method's caller, or to something above it.
    //
    // If the returned state is something other than TAG_RETURN, _or_ the 'return' is not intended to be handled in
    // this frame, then we will continue the transfer below via EC_JUMP_TAG.
    else if (state == TAG_RETURN) {
        const struct vm_throw_data *const err = (struct vm_throw_data *)ec->errinfo;
        const rb_control_frame_t *const escape_cfp = THROW_DATA_CATCH_FRAME(err);

        // The use of RUBY_VM_PREVIOUS_CONTROL_FRAME here is strange, but I think it is correct. If we are calling a
        // compiled lambda, and a return under the lambda resolves to return from the lambda, then the escape CFP will
        // be the lambda's control frame, but the catch here is happening in the lambda's caller's frame's context.
        //
        // Note that interpreted lambdas called from here follow a different codepath: the rb_vm_exec for the lambda's
        // block frame will catch the TAG_RETURN and simply return the value to us.
        if (save_cfp == RUBY_VM_PREVIOUS_CONTROL_FRAME(escape_cfp)) {
            rb_vm_rewind_cfp(ec, save_cfp);

            // Zero out state to indicate to the logic below that we have processed the return and do not need to
            // re-raise.
            state = 0;

            ec->tag->state = TAG_NONE;
            ec->errinfo = Qnil;
            retval = THROW_DATA_VAL(err);
        }
    }

    // If we have fallen through to this point (whether or not we caught an EC_JUMP_TAG), we will pop the tag that we
    // pushed above.
    EC_POP_TAG();

    // If state is still nonzero, we did not handle the jump, so it must have been intended for someone above us. Thus
    // we "re-raise" the jump.
    if (state) {
        EC_JUMP_TAG(ec, state);
    }

    // If we get this far, this jump was ours to handle, so we just return the thrown retval.
    return retval;
}

// This is a version of vm_call_method_each_type that differs in the handling of refined methods. As we know that we're
// calling from a CFUNC context, using vm_env_cref will cause a runtime crash, so instead we inline the behavior of
// `vm_call0_body` for refined methods,
// https://github.com/ruby/ruby/blob/5445e0435260b449decf2ac16f9d09bae3cafe72/vm_insnhelper.c#L2911-L2993
static VALUE sorbet_vm_call_method_each_type(rb_execution_context_t *ec, rb_control_frame_t *cfp,
                                             struct rb_calling_info *calling, struct rb_call_data *cd) {
    const struct rb_call_info *ci = &cd->ci;
    struct rb_call_cache *cc = &cd->cc;

// begin differences from vm_call_method_each_type
again:
    // end differences from vm_call_method_each_type

    switch (cc->me->def->type) {
        case VM_METHOD_TYPE_ISEQ:
            CC_SET_FASTPATH(cc, vm_call_iseq_setup, TRUE);
            return vm_call_iseq_setup(ec, cfp, calling, cd);

        case VM_METHOD_TYPE_NOTIMPLEMENTED:
        case VM_METHOD_TYPE_CFUNC:
            CC_SET_FASTPATH(cc, vm_call_cfunc, TRUE);
            return vm_call_cfunc(ec, cfp, calling, cd);

        case VM_METHOD_TYPE_SORBET:
            CC_SET_FASTPATH(cc, vm_call_sorbet, TRUE);
            return vm_call_sorbet_maybe_setup_fastpath(ec, cfp, calling, cd);

        case VM_METHOD_TYPE_ATTRSET:
            CALLER_SETUP_ARG(cfp, calling, ci);
            if (calling->argc == 1 && calling->kw_splat && RHASH_EMPTY_P(cfp->sp[-1])) {
                rb_warn_keyword_to_last_hash(ec, calling, ci, NULL);
            } else {
                CALLER_REMOVE_EMPTY_KW_SPLAT(cfp, calling, ci);
            }

            rb_check_arity(calling->argc, 1, 1);
            cc->aux.index = 0;
            CC_SET_FASTPATH(cc, vm_call_attrset, !((ci->flag & VM_CALL_ARGS_SPLAT) || (ci->flag & VM_CALL_KWARG)));
            return vm_call_attrset(ec, cfp, calling, cd);

        case VM_METHOD_TYPE_IVAR:
            CALLER_SETUP_ARG(cfp, calling, ci);
            CALLER_REMOVE_EMPTY_KW_SPLAT(cfp, calling, ci);
            rb_check_arity(calling->argc, 0, 0);
            cc->aux.index = 0;
            CC_SET_FASTPATH(cc, vm_call_ivar, !(ci->flag & VM_CALL_ARGS_SPLAT));
            return vm_call_ivar(ec, cfp, calling, cd);

        case VM_METHOD_TYPE_MISSING:
            cc->aux.method_missing_reason = 0;
            CC_SET_FASTPATH(cc, vm_call_method_missing, TRUE);
            return vm_call_method_missing(ec, cfp, calling, cd);

        case VM_METHOD_TYPE_BMETHOD:
            CC_SET_FASTPATH(cc, vm_call_bmethod, TRUE);
            return vm_call_bmethod(ec, cfp, calling, cd);

        case VM_METHOD_TYPE_ALIAS:
            CC_SET_ME(cc, aliased_callable_method_entry(cc->me));
            VM_ASSERT(cc->me != NULL);
            return sorbet_vm_call_method_each_type(ec, cfp, calling, cd);

        case VM_METHOD_TYPE_OPTIMIZED:
            switch (cc->me->def->body.optimize_type) {
                case OPTIMIZED_METHOD_TYPE_SEND:
                    CC_SET_FASTPATH(cc, vm_call_opt_send, TRUE);
                    return vm_call_opt_send(ec, cfp, calling, cd);
                case OPTIMIZED_METHOD_TYPE_CALL:
                    // begin differences from vm_call_method_each_type
                    // We use a patched version of sorbet_vm_call_opt_call which catches and handles returns from
                    // lambdas.
                    CC_SET_FASTPATH(cc, sorbet_vm_call_opt_call, TRUE);
                    return sorbet_vm_call_opt_call(ec, cfp, calling, cd);
                    // end differences from vm_call_method_each_type
                case OPTIMIZED_METHOD_TYPE_BLOCK_CALL:
                    CC_SET_FASTPATH(cc, vm_call_opt_block_call, TRUE);
                    return vm_call_opt_block_call(ec, cfp, calling, cd);
                default:
                    rb_bug("vm_call_method: unsupported optimized method type (%d)", cc->me->def->body.optimize_type);
            }

        case VM_METHOD_TYPE_UNDEF:
            break;

        case VM_METHOD_TYPE_ZSUPER:
            return vm_call_zsuper(ec, cfp, calling, cd, RCLASS_ORIGIN(cc->me->defined_class));

        case VM_METHOD_TYPE_REFINED: {
            // begin differences from vm_call_method_each_type

            // This is the case for refined methods in `vm_call0_body`, inlined into the implementation of
            // `vm_call_method_each_type`, as that implementation assumed that it was called from an interpreted
            // context, and attempts to project out the cref from the method entry, which does not have a valid cref in
            // a compiled context.
            // https://github.com/ruby/ruby/blob/5445e0435260b449decf2ac16f9d09bae3cafe72/vm_eval.c#L173-L198

            if (cc->me->def->body.refined.orig_me) {
                CC_SET_ME(cc, refined_method_callable_without_refinement(cc->me));
                goto again;
            }

            VALUE super_class = RCLASS_SUPER(cc->me->defined_class);
            if (super_class) {
                CC_SET_ME(cc, rb_callable_method_entry(super_class, ci->mid));
                if (cc->me) {
                    RUBY_VM_CHECK_INTS(ec);
                    goto again;
                }
            }

            // We call vm_call_method_nome instead of `missing_method` like `vm_call0_body`.
            // This matches the behavior of vm_call_method_each_type:
            // https://github.com/ruby/ruby/blob/5445e0435260b449decf2ac16f9d09bae3cafe72/vm_insnhelper.c#L2989
            return vm_call_method_nome(ec, cfp, calling, cd);

            // end differences from vm_call_method_each_type
        }
    }

    rb_bug("vm_call_method: unsupported method type (%d)", cc->me->def->type);
}

// This is a version of vm_call_method that dispatches to `sorbet_vm_call_method_each_type` instead.
// https://github.com/ruby/ruby/blob/5445e0435260b449decf2ac16f9d09bae3cafe72/vm_insnhelper.c#L3017-L3070
static inline VALUE sorbet_vm_call_method(rb_execution_context_t *ec, rb_control_frame_t *cfp,
                                          struct rb_calling_info *calling, struct rb_call_data *cd) {
    const struct rb_call_info *ci = &cd->ci;
    struct rb_call_cache *cc = &cd->cc;

    VM_ASSERT(callable_method_entry_p(cc->me));

    if (cc->me != NULL) {
        switch (METHOD_ENTRY_VISI(cc->me)) {
            case METHOD_VISI_PUBLIC: /* likely */
                // begin differences from vm_call_method_each_type
                return sorbet_vm_call_method_each_type(ec, cfp, calling, cd);
                // begin differences from vm_call_method_each_type

            case METHOD_VISI_PRIVATE:
                if (!(ci->flag & VM_CALL_FCALL)) {
                    enum method_missing_reason stat = MISSING_PRIVATE;
                    if (ci->flag & VM_CALL_VCALL)
                        stat |= MISSING_VCALL;

                    cc->aux.method_missing_reason = stat;
                    CC_SET_FASTPATH(cc, vm_call_method_missing, TRUE);
                    return vm_call_method_missing(ec, cfp, calling, cd);
                }
                // begin differences from vm_call_method_each_type
                return sorbet_vm_call_method_each_type(ec, cfp, calling, cd);
                // end differences from vm_call_method_each_type

            case METHOD_VISI_PROTECTED:
                if (!(ci->flag & VM_CALL_OPT_SEND)) {
                    if (!rb_obj_is_kind_of(cfp->self, cc->me->defined_class)) {
                        cc->aux.method_missing_reason = MISSING_PROTECTED;
                        return vm_call_method_missing(ec, cfp, calling, cd);
                    } else {
                        /* caching method info to dummy cc */
                        VM_ASSERT(cc->me != NULL);
                        if (ci->flag & VM_CALL_KWARG) {
                            struct rb_kwarg_call_data *kcd = (void *)cd;
                            struct rb_kwarg_call_data cd_entry = *kcd;
                            // begin differences from vm_call_method_each_type
                            return sorbet_vm_call_method_each_type(ec, cfp, calling, (void *)&cd_entry);
                            // end differences from vm_call_method_each_type
                        } else {
                            struct rb_call_data cd_entry = *cd;
                            // begin differences from vm_call_method_each_type
                            return sorbet_vm_call_method_each_type(ec, cfp, calling, &cd_entry);
                            // end differences from vm_call_method_each_type
                        }
                    }
                }
                // begin differences from vm_call_method_each_type
                return sorbet_vm_call_method_each_type(ec, cfp, calling, cd);
                // end differences from vm_call_method_each_type

            default:
                rb_bug("unreachable");
        }
    } else {
        return vm_call_method_nome(ec, cfp, calling, cd);
    }
}

// This is a version of `vm_sendish` that's always called from a CFUNC context. We also assume that MJIT is disabled,
// and inline some behavior as a result.
// https://github.com/ruby/ruby/blob/5445e0435260b449decf2ac16f9d09bae3cafe72/vm_insnhelper.c#L3998-L4056
static inline VALUE sorbet_vm_sendish(struct rb_execution_context_struct *ec, struct rb_control_frame_struct *reg_cfp,
                                      struct rb_call_data *cd, VALUE block_handler) {
    CALL_INFO ci = &cd->ci;
    CALL_CACHE cc = &cd->cc;
    VALUE val;
    int argc = ci->orig_argc;
    VALUE recv = TOPN(argc);
    struct rb_calling_info calling;

    calling.block_handler = block_handler;
    calling.kw_splat = IS_ARGS_KW_SPLAT(ci) > 0;
    calling.recv = recv;
    calling.argc = argc;

    // inlined instead of called via vm_search_method_wrap
    vm_search_method(cd, recv);

    // We need to avoid using `vm_call_general`, and instead call `sorbet_vm_call_general`. See the comments in
    // `sorbet_vm_call_method_each_type` for more information.
    //
    // Uses UNLIKELY to make the fast path of "call cache hit" faster
    if (UNLIKELY(cc->call == vm_call_general)) {
        val = sorbet_vm_call_method(ec, GET_CFP(), &calling, cd);
    } else {
        val = cc->call(ec, GET_CFP(), &calling, cd);
    }

    if (val != Qundef) {
        return val; /* CFUNC normal return */
    } else {
        RESTORE_REGS(); /* CFP pushed in cc->call() */
    }

    return Qundef;
}

// This send primitive assumes that all argumenst have been pushed to the ruby stack, and will invoke the vm machinery
// to execute the send.
VALUE sorbet_callFuncWithCache(struct FunctionInlineCache *cache, VALUE bh) {
    rb_execution_context_t *ec = GET_EC();
    rb_control_frame_t *cfp = ec->cfp;

    VALUE val = sorbet_vm_sendish(ec, cfp, (struct rb_call_data *)&cache->cd, bh);
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
    const VALUE duplicate_input_arrays = Qtrue;
    return vm_splat_array(duplicate_input_arrays, thing);
}

VALUE sorbet_vm_check_match_array(rb_execution_context_t *ec, VALUE target, VALUE pattern) {
    int flag = VM_CHECKMATCH_ARRAY | VM_CHECKMATCH_TYPE_CASE;
    return vm_check_match(ec, target, pattern, flag);
}

VALUE sorbet_vm_getivar(VALUE obj, ID id, IVC ic) {
    /* cf. getinstancevariable in insns.def and vm_getinstancevariable */
    struct rb_call_cache *cc = 0;
    int is_attr = 0;
    return vm_getivar(obj, id, ic, cc, is_attr);
}

void sorbet_vm_setivar(VALUE obj, ID id, VALUE val, IVC ic) {
    /* cf. setinstancevariable in insns.def and vm_setinstancevariable */
    struct rb_call_cache *cc = 0;
    int is_attr = 0;
    vm_setivar(obj, id, val, ic, cc, is_attr);
}

void sorbet_throwReturn(rb_execution_context_t *ec, VALUE retval) {
    VALUE v = vm_throw(ec, ec->cfp, TAG_RETURN, retval);

    ec->errinfo = v;
    EC_JUMP_TAG(ec, ec->tag->state);
}

struct rfb_status {
    VALUE return_value;
    bool was_thrown;
};

struct rfb_status sorbet_vm_return_from_block_wrapper(int argc, VALUE *argv, VALUE recv,
                                                      rb_control_frame_t *volatile cfp, rb_sorbet_func_t wrapped) {
    enum ruby_tag_type state;
    rb_execution_context_t *volatile ec = GET_EC();
    struct rfb_status status;
    status.return_value = Qundef;
    status.was_thrown = false;

    EC_PUSH_TAG(ec);

    if ((state = EC_EXEC_TAG()) == 0) {
        status.return_value = wrapped(argc, argv, recv, cfp);
        status.was_thrown = false;
    } else {
        if (state == TAG_RETURN) {
            const struct vm_throw_data *const err = (struct vm_throw_data *)ec->errinfo;
            const rb_control_frame_t *const escape_cfp = err->catch_frame;

            if (cfp == escape_cfp) {
                rb_vm_rewind_cfp(ec, cfp);
                state = TAG_NONE;
                ec->errinfo = Qnil;
                status.return_value = err->throw_obj;
                status.was_thrown = true;
            }
        }
    }

    EC_POP_TAG();

    if (state != TAG_NONE) {
        rb_ec_tag_jump(ec, state);
    }

    return status;
}

// The functions for reading and writing locals are intentionally duplicated here
// from the codegen payload, but are marked static so the codegen payload's versions
// are always used for the shared objects generated by the compiler.
#define SORBET_INLINE __attribute__((always_inline))

SORBET_INLINE
static int computeLocalIndex(long index) {
    // Local offset calculation needs to take into account the fixed values that
    // are present on the stack:
    // https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/compile.c#L1509
    return index + VM_ENV_DATA_SIZE;
}

SORBET_INLINE
static VALUE sorbet_readLocal(rb_control_frame_t *cfp, long index, long level) {
    int offset = computeLocalIndex(index);
    return *(vm_get_ep(cfp->ep, level) - offset);
}
SORBET_INLINE
static void sorbet_writeLocal(rb_control_frame_t *cfp, long index, long level, VALUE value) {
    int offset = computeLocalIndex(index);
    vm_env_write(vm_get_ep(cfp->ep, level), -offset, value);
}

typedef VALUE (*ExceptionFFIType)(VALUE **pc, VALUE closure, rb_control_frame_t *);

SORBET_INLINE
static void sorbet_raiseIfNotNil(VALUE exception) {
    if (exception == RUBY_Qnil) {
        return;
    }

    rb_exc_raise(exception);
}
VALUE sorbet_run_exception_handling(rb_execution_context_t *volatile ec, volatile ExceptionFFIType body,
                                    VALUE **volatile pc,
                                    // The locals offset for the body.
                                    volatile VALUE methodClosure, rb_control_frame_t *volatile cfp,
                                    // May be nullptr.
                                    volatile ExceptionFFIType handlers,
                                    // May be nullptr.
                                    volatile ExceptionFFIType elseClause,
                                    // May be nullptr.
                                    volatile ExceptionFFIType ensureClause,
                                    // The special value indicating that we need to retry.
                                    volatile VALUE retrySingleton, volatile long exceptionValueIndex,
                                    volatile long exceptionValueLevel) {
    // `volatile` is not used in polite C programming, but here it's very important:
    // it ensures that the requisite variables are stored in memory across the setjmp
    // below and therefore will still be valid upon a return via longjmp.  All such
    // variables that need to be live "on both sides" of the setjmp need to be
    // declared with `volatile`.
    //
    // Temporary variables used on the "longjmp side" only can be declared as normal.
    volatile VALUE executionResult = Qundef;
    volatile VALUE previousException = ec->errinfo;
    volatile VALUE bodyException = Qnil;
    volatile VALUE handlerException = Qnil;
    volatile enum {
        RunningBody,
        RunningHandlers,
    } state = RunningBody;
    volatile enum ruby_tag_type nleType;

    EC_PUSH_TAG(ec);

    if ((nleType = EC_EXEC_TAG()) == TAG_NONE) {
    execute_body:
        state = RunningBody;
        // tag.state will have been reset appropriately if we got here via `retry`,
        // but not this variable.
        nleType = TAG_NONE;

        // Clear out the local variable shared across exception handling regions
        // where we store the current exception value.  Note that we do not pull
        // out a pointer to the local variable and use that throughout this function,
        // as it's possible that the locals may shift from the Ruby stack to the
        // heap during the execution of the exception-handling region.
        sorbet_writeLocal(cfp, exceptionValueIndex, exceptionValueLevel, Qnil);

        // We're also done with whatever exceptions might have gotten thrown along the way.
        bodyException = Qnil;
        handlerException = Qnil;

        // Likewise with any results we accumulated along the way.
        executionResult = Qundef;

        // Run the body.
        executionResult = body(pc, methodClosure, cfp);

        // If we get to this point, then we know the body has succeeded without throwing
        // an exception.  We may need to run the `else` handler if the body didn't
        // return a value that we need to propagate.
        if (executionResult == Qundef) {
            state = RunningHandlers;

            executionResult = elseClause(pc, methodClosure, cfp);
        }
        rb_set_errinfo(previousException);
    } else {
        // If we get here, setjmp has returned a non-zero value.

        // This case is the "obvious" case: something in the body threw; we need to handle
        // exceptions directly here.
        if (state == RunningBody) {
            // We're handling things very similarly to rb_rescue2/rb_vrescue2:
            // <add-link-here>
            //
            // The significant difference from that function is that we're handling all
            // the non-local exits directly.

            // Whatever kind of non-local exit we have, we need to make sure that
            // the Ruby control frame for the handler (or the ensure) we're going
            // to run lives directly under whatever frame we started this process with.
            rb_vm_rewind_cfp(ec, cfp);

            if (nleType != TAG_RAISE) {
                // Any other kind of non-local exit will skip the rescue/else handlers.
                goto execute_ensure;
            }

            // rb_rescue2/rb_vrescue2 would check ec->errinfo here to determine if it
            // was the "right" kind of error.  Sorbet has already generated code to check
            // the exception value for us, we can dispatch directly to the handlers here,
            // which avoids a little bit of overhead.  But we do need to tell the
            // handlers what the exception value *is*.
            bodyException = ec->errinfo;

            // Any exception that got thrown needs to be set for the handler.
            sorbet_writeLocal(cfp, exceptionValueIndex, exceptionValueLevel, bodyException);

            ExceptionFFIType handler = (bodyException != Qnil) ? handlers : elseClause;

            // Indicate that we are running the appropriate handler.
            nleType = TAG_NONE;
            state = RunningHandlers;

            executionResult = handler(pc, methodClosure, cfp);

            // If execution has reached this point, the handler did not throw any
            // kind of non-local exit.
            if (bodyException != Qnil && executionResult == retrySingleton) {
                goto execute_body;
            }
        } else {
            // This case is the non-obvious case: something in the handler we were executing
            // threw.  We need to clean up and execute the ensure.

            rb_vm_rewind_cfp(ec, cfp);

            // TODO(froydnj): this is where we would handle TAG_RETRY if we were implementing
            // `retry` in terms of Ruby's non-local exit handling rather than our current
            // retry singleton mechanism.

            if (nleType != TAG_RAISE) {
                goto execute_ensure;
            }

            handlerException = ec->errinfo;
            sorbet_writeLocal(cfp, exceptionValueIndex, exceptionValueLevel, handlerException);
        }

        // We need to determine what the value of the "current" exception is for the
        // ensure handler.  There are three possibilities, in order of preference:
        //
        // 1. An exception raised by the rescue block.
        // 2. The original exception raised by the body, that wasn't handled by the rescue block.
        // 3. The ambient previous exception present prior to exception handling.
        VALUE postRescueExceptionContext;

        if (handlerException != Qnil) {
            // Case 1.
            postRescueExceptionContext = handlerException;
        } else {
            // The exception value local was set for the purposes of the rescue
            // handler (it will be `nil` if the body didn't raise an exception).
            // The rescue handler will have nil'd out this variable if an
            // appropriate handler was run.
            VALUE bodyException = sorbet_readLocal(cfp, exceptionValueIndex, exceptionValueLevel);
            if (bodyException != Qnil) {
                // Case 2.
                postRescueExceptionContext = bodyException;
            } else {
                // Case 3.
                postRescueExceptionContext = previousException;
            }
        }

        rb_set_errinfo(postRescueExceptionContext);
    }

execute_ensure:
    // However we arrived at this state, we are done with our entry on the tag stack.
    EC_POP_TAG();

    // Ruby's running of ensure handlers passes in rb_errinfo() as an argument on the
    // stack ; ensure handlers then exit via the bytecode instruction `throw 0 local[0]`,
    // which eventually winds its way into vm_throw_continue.  We don't codegen things
    // that way -- perhaps we should, but this emulates that behavior.
    VALUE pendingException = ec->errinfo;

    // Running the ensure handler will push an VM_FRAME_MAGIC_RESCUE control frame
    // and also have the side effect of clearing out ec->errinfo and ec->tag->state.
    VALUE ensureResult = ensureClause(pc, methodClosure, cfp);

    // Anything the ensure returned takes precedence over a pending non-local exit.
    if (ensureResult != Qundef) {
        return ensureResult;
    }

    // Put the error back.  We already have nleType, which was ec->tag->state prior
    // to running the ensure handler, for resetting ec->tag->state if appropriate.
    ec->errinfo = pendingException;

    // If we had a non-local exit from the region above, propagating that takes precedence
    // over whatever the ensure handler did.
    //
    // cf. EC_JUMP_TAG for the code here.
    if (nleType != TAG_NONE) {
        ec->tag->state = nleType;
        RUBY_LONGJMP(ec->tag->buf, 1);
    }

    // executionResult is the result from running either the body or the handlers,
    // depending on what path we took through the code above.

    sorbet_raiseIfNotNil(handlerException);
    sorbet_raiseIfNotNil(sorbet_readLocal(cfp, exceptionValueIndex, exceptionValueLevel));

    return executionResult;
}

VALUE sorbet_vm_callBlock(rb_control_frame_t *cfp, int argc, const VALUE *const restrict argv, int kw_splat) {
    // cf. https://github.com/ruby/ruby/blob/ruby_2_7/vm_insnhelper.c#L3990-L3995
    if (UNLIKELY(VM_CF_BLOCK_HANDLER(cfp) == VM_BLOCK_HANDLER_NONE)) {
        rb_vm_localjump_error("no block given (yield)", Qnil, 0);
    }
    return rb_yield_values_kw(argc, argv, kw_splat);
}

// This is a version of rb_iterate specialized to the case where we know the block is non-null and its arity.
VALUE sorbet_rb_iterate(VALUE (*it_proc)(VALUE), VALUE data1, rb_block_call_func_t bl_proc, int minArgs, int maxArgs,
                        VALUE data2) {
    rb_execution_context_t *ec = GET_EC();
    rb_control_frame_t *const cfp = ec->cfp;

    enum ruby_tag_type state;
    volatile VALUE retval = Qnil;

    const struct vm_ifunc *const ifunc = rb_vm_ifunc_new(bl_proc, (void *)data2, minArgs, maxArgs);

    EC_PUSH_TAG(ec);
    state = EC_EXEC_TAG();
    if (state == 0) {
        // clang-format off
iter_retry:

        {
            VALUE block_handler;

            struct rb_captured_block *captured = VM_CFP_TO_CAPTURED_BLOCK(cfp);
            captured->code.ifunc = ifunc;
            block_handler = VM_BH_FROM_IFUNC_BLOCK(captured);
            vm_passed_block_handler_set(ec, block_handler);
        }

        retval = (*it_proc)(data1);
        // clang-format on
    } else if (state == TAG_BREAK || state == TAG_RETRY) {
        const struct vm_throw_data *const err = (struct vm_throw_data *)ec->errinfo;
        const rb_control_frame_t *const escape_cfp = THROW_DATA_CATCH_FRAME(err);

        if (cfp == escape_cfp) {
            rb_vm_rewind_cfp(ec, cfp);

            state = 0;
            ec->tag->state = TAG_NONE;
            ec->errinfo = Qnil;

            if (state == TAG_RETRY)
                goto iter_retry;
            retval = THROW_DATA_VAL(err);
        } else if (0) {
            SDR();
            fprintf(stderr, "%p, %p\n", (void *)cfp, (void *)escape_cfp);
        }
    }
    EC_POP_TAG();

    if (state) {
        EC_JUMP_TAG(ec, state);
    }
    return retval;
}
