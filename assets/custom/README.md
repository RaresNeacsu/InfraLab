# Custom scenarios

These are the project's original, manually defined synthetic scenarios.

- `requests_normal.json`: normal traffic, 12 requests.
- `requests_burst.json`: burst traffic, 19 requests.
- `requests_overload.json`: sustained overload, 32 requests.
- `servers.json`: the default three-server configuration.
- `servers_expanded.json`: the configuration with an additional server.

These datasets are not measured production traffic. The application uses the
normal requests and default servers when no arguments are supplied.
