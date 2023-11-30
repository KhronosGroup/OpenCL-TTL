Import and Export Transactions
==============================

The third layer of TTL deals with transactions that copy tensors from global to
local memory and back, referred to as import and export, respectively. These
transactions are asynchronous and correspond to ``async_work_group_copy()`` builtin
functions of OpenCL and their use of ``event_t``. Similar to OpenCL, in TTL one
``event_t`` can serve multiple transactions, and it is possible to wait on multiple
events. Unlike OpenCL, every import and export in TTL must be provided a
non-null ``event_t``, which can be produced by ``TTL_get_event()``.

.. code-block:: c

    TTL_event_t TTL_get_event(); // Initialize an event.

    // Import the data in external_tensor to internal_tensor.
    // The transaction is added to the event e.
    void TTL_import(TTL_int_tensor_t internal_tensor, TTL_ext_tensor_t external_tensor, TTL_event_t *e);

    // Export the data in internal_tensor to external_tensor.
    // The transaction is added to the event e.
    void TTL_export(TTL_int_tensor_t internal_tensor, TTL_ext_tensor_t external_tensor, TTL_event_t *e);

    void TTL_wait(int num_events, TTL_event_t *events); // Wait for first num_events in events array

TTL_blocking_import/export can be used to issue a blocking transaction, i.e.,
get an event, issue a transaction and immediately wait for its completion:

.. code-block:: c

    void TTL_blocking_import(TTL_int_tensor_t internal_tensor, TTL_ext_tensor_t external_tensor);
    void TTL_blocking_export(TTL_int_tensor_t internal_tensor, TTL_ext_tensor_t external_tensor);