**********************
Memory Management Note
**********************

In nginx, memory management is implemented using memory pools, which is implemented using
singly linked list. an has several benefits:

    - minimize memory leak
    - reduce the number of malloc-related system calls, thus reduce fragmentations
    - improve memory access: memory addresses are aligned and there is a memory paging
      mechanism improving CPU page hits.

Source files: ``ngx_palloc.h, ngx_palloc.c, ngx_alloc.h, ngx_alloc.c``.

.. note::

    Different pool size may result in different memory usage. However, the less system malloc,
    the less memory fragmentations, which means the less memory usage.

.. rubric:: Footnotes

.. [#] `What is memory mapping <http://ecomputernotes.com/fundamental/input-output-and-memory/memory-mapping>`_
