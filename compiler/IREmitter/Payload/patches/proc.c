/* method_owner, the backing function for Method#owner, is not exported.  */
VALUE sorbet_vm_method_owner(VALUE obj) {
    return method_owner(obj);
}
