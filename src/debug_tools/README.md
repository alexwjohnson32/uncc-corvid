# `debug_tools`

This directory and these tools exist simply to assit in development and debugging, and should not be used outside of those contexts, or expected to do anything extraordinary.

## `testing_client`

This is a simple websocket client that accepts an `address`, `port`, and `path` for a particular websockt server address, connects to it, and allows you to write messages through the CLI to send to the server.

## `testing_server`

This is a simple websocket server that launches at the specified `address`, `port` and `path`. It stays alive forver unless a fatal error occurs or you kill it manually (like with `ctrl+c`). It listens for messages and prints them to the CLI.