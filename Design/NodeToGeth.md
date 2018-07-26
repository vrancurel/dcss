# geth communication

The communication between `DCSS`'s nodes and `geth` is done through
JSON-RPC.
The C++ client, `GethClient`, is automatically generated from `geth_spec.json`
using `jsonrpcstub`.

There is a single instance of `GethClient`, shared by all the nodes, in the
global configuration of `DCSS`.

Each `DCSS` node has an Ethereum account (created when the node is
spawned/respawned¹).
The node can execute contract on the blockchain² to buy storage and PUT/GET
tokens.

Note that `geth` must be started with its RPC server and should expose at least
the interfaces `eth` and `personal`³.

## Notes

¹: for now, we always create a brand new account (with no fund) when we create a
node and the passphrase of the account is the node's ID (not secure). We may
want to reuse existing account instead of creating a brand new one (with no
fund) at each spawn/respawn of the network…

²: the current implementation is blocking and use a busy loop, to scale we may
need to switch to an async/event-driven model.

³: from the doc: "Please note, offering an API over the HTTP (rpc) interfaces
will give everyone access to the APIs who can access this interface. Be careful
which APIs you enable"
