*****************
Load Balance Note
*****************

Load load balance methods nginx supports includes:

    - Round Robin
    - Least Connections
    - IP Hash
    - Generic Hash
    - ...

By default, nginx distributes requests among the servers in the group according to their weights
using the Round Robin method. The ``weight`` parameter to the server directive sets the weight of
a server; the default is 1:

.. code-block:: none

    upstream backend {
        server backend1.example.com weight=5;
        server backend2.example.com;
        server 192.0.0.1 backup;
    }

In the example, ``backend1.example.com`` has weight 5; the other two servers have the default weight (1),
but the one with IP address ``192.0.0.1`` is marked as a ``backup`` server and does not receive requests
unless both of the other servers are unavailable. With this configuration of weights, out of every 6 requests,
5 are sent to ``backend1.example.com`` and 1 to ``backend2.example.com.``

Nginx load balance implementation can be found in:

    - ngx_http_upstream_round_robin.h, ngx_http_upstream_round_robin.c


Some code sinppets:

.. code-block:: c
    :caption: Round Robin method

    ngx_int_t ngx_http_upstream_get_round_robin_peer(ngx_peer_connection_t *pc, void *data)
    {
        // ...
        if (peers->single)
        {
            peer = peers->peer;
            rrp->current = peer;
        }
        else
        {
            /* there are several peers */
            peer = ngx_http_upstream_get_peer(rrp);
            if (peer == NULL) {
                goto failed;
            }
        }

        peer->conns++;

        return NGX_OK;

    failed:

        if (peers->next)
        {
            // change to backup servers
            rrp->peers = peers->next;
            ngx_int_t rc = ngx_http_upstream_get_round_robin_peer(pc, rrp);
            if (rc != NGX_BUSY) {
                return rc;
            }
        }

        return NGX_BUSY;
    }

    static ngx_http_upstream_rr_peer_t* ngx_http_upstream_get_peer(ngx_http_upstream_rr_peer_data_t *rrp)
    {
        // ...
        for (peer = rrp->peers->peer, i = 0;
             peer;
             peer = peer->next, i++)
        {
            // ...
            peer->current_weight += peer->effective_weight;
            total += peer->effective_weight;

            if (best == NULL || peer->current_weight > best->current_weight) {
                best = peer;
                p = i;
            }
        }

        rrp->current = best;

        // A trick comes here
        best->current_weight -= total;

        if (now - best->checked > best->fail_timeout) {
            best->checked = now;
        }

        return best;
    }

.. code-block:: c
    :caption: Least Connections method

    static ngx_int_t ngx_http_upstream_get_least_conn_peer(ngx_peer_connection_t *pc, void *data)
    {
        for (peer = peers->peer, i = 0;
             peer;
             peer = peer->next, i++)
        {
            // ...

            /*
             * select peer with least number of connections; if there are
             * multiple peers with the same number of connections, select
             * based on round-robin
             */

            if (best == NULL
                || peer->conns * best->weight < best->conns * peer->weight)
            {
                best = peer;
                many = 0;
                p = i;

            } else if (peer->conns * best->weight == best->conns * peer->weight) {
                many = 1;
            }
        }

        // ...

        if (many)
        {
            for (peer = best, i = p;
                 peer;
                 peer = peer->next, i++)
            {
                // ...

                if (peer->conns * best->weight != best->conns * peer->weight) {
                    continue;
                }

                peer->current_weight += peer->effective_weight;
                total += peer->effective_weight;

                if (peer->effective_weight < peer->weight) {
                    peer->effective_weight++;
                }

                if (peer->current_weight > best->current_weight) {
                    best = peer;
                    p = i;
                }
            }
        }

        best->current_weight -= total;
        rrp->current = best;
        return NGX_OK;

        // ...
    }


.. rubric:: Footnotes

.. [#] `Http load balancer <https://docs.nginx.com/nginx/admin-guide/load-balancer/http-load-balancer/>`_
