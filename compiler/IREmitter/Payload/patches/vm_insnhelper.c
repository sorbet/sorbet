
void sorbet_vm_search_method_wrap(const struct rb_control_frame_struct *reg_cfp, struct rb_call_data *cd, VALUE recv) {
    return vm_search_method_wrap(reg_cfp, cd, recv);
}

VALUE sorbet_vm_sendish(struct rb_execution_context_struct *ec, struct rb_control_frame_struct *reg_cfp,
                        struct rb_call_data *cd, VALUE block_handler,
                        void (*method_explorer)(const struct rb_control_frame_struct *reg_cfp, struct rb_call_data *cd,
                                                VALUE recv)) {
    return vm_sendish(ec, reg_cfp, cd, block_handler, method_explorer);
}
