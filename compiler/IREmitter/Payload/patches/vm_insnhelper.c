
void sorbet_vm_search_method_wrap(const struct rb_control_frame_struct *reg_cfp, struct rb_call_data *cd, VALUE recv) {
    return vm_search_method_wrap(reg_cfp, cd, recv);
}

VALUE sorbet_vm_sendish(struct rb_execution_context_struct *ec, struct rb_control_frame_struct *reg_cfp,
                        struct rb_call_data *cd, VALUE block_handler,
                        void (*method_explorer)(const struct rb_control_frame_struct *reg_cfp, struct rb_call_data *cd,
                                                VALUE recv)) {
    return vm_sendish(ec, reg_cfp, cd, block_handler, method_explorer);
}

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
    rb_vm_pop_frame(GET_EC());
}
