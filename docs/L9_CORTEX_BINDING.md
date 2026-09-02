# HAKUI L9 — Spiral Cortex Binding

L9 binds the visible L8 `SpiralPresence` to the real Spiral Ether AI runtime without transferring HAKUI simulation authority to AI code.

## Runtime topology

```text
HAKUI GameRuntime                         Spiral Ether AI
(authoritative reality)                   (cognition)
        |
   HakuiSnapshot
        |
   HakuiAdapter
   read-only
        |
 SpiralPresence ---- status ---- SpiralCortexClient
                                     |
                          loopback 127.0.0.1:47691
                          SPIRAL_HAKUI/1
                                     |
                              SpiralHakuiCortex
                                     |
                      spiral::ether_ai::Runtime
                             HostKind::Hakui
```

`SpiralCortexClient` sends owned copies of L6 snapshot truth. The wire context explicitly declares `authority=read_only`, `policy.no_world_mutation=true`, `policy.no_player_control=true`, and `policy.no_interaction_execution=true`.

The cortex response is text only. The native client posts it through the existing `SystemAI` social presentation path. L9 does not parse the response into movement, spawning, combat, interaction, inventory, world, or Spiral state commands.

## Companion runtime

The companion implementation lives on `spiraletech/spiralos-ai-genius` branch `spiral/hakui-l9-cortex-bridge`. Its `SpiralHakuiCortex` executable hosts the existing `spiral::ether_ai::Runtime` with `hakui_host()` and listens only on IPv4 loopback.

Default endpoint: `127.0.0.1:47691`.

Optional model startup:

```text
SpiralHakuiCortex.exe --model C:\path\to\model.gguf
```

or set `SPIRAL_HAKUI_MODEL` before starting the bridge.

Without a bridge, HAKUI still boots and plays normally. The L8 node reports `CORTEX // OFFLINE // START BRIDGE`; this is an optional cognition capability, not a gameplay dependency.

## Native interaction

1. Start `SpiralHakuiCortex.exe`.
2. Start HAKUI.
3. Walk within the authored `SPIRAL PRESENCE NODE` radius.
4. Press Enter and type a message.
5. Press Enter to send it to the cortex.
6. HAKUI freezes the current L6 observation packet for that request; the worker thread sends only that copy plus the human prompt.
7. The reply returns to the main thread mailbox and appears as a `SystemAI` message.

Ordinary Enter chat away from the node remains ordinary local avatar chat.

## Concurrency boundary

The model request never holds references to `GameRuntime`, `PlayerState`, `BlackRoom`, interaction services, renderer state, or the resident adapter. A detached request worker owns only a copyable stateless client, an owned `HakuiSnapshot`, the prompt string, and a shared mailbox. HAKUI continues simulation/rendering while the cortex thinks.

## L9 invariants

- HAKUI owns world and player authority.
- L7 remains the only semantic perception boundary.
- L8 remains presentation-only presence.
- L9 transports frozen observation + human text to cognition and returns text only.
- Cortex offline must never prevent HAKUI boot or gameplay.
- No response-to-action execution exists in L9.

A future action layer must introduce an explicit permissioned command contract rather than bypassing this boundary.
