import asyncio
import websockets
import socket
import cv2
import numpy as np

class Messages:
    END_CONNECTION = "[END_CONNECTION]"
    CONFIG_PREFIX = "[CONFIG]"
    COMMAND_PREFIX = "[COMMAND]"
    STREAM_PREFIX = "[STREAM]"

class StreamStates:
    PAUSE = "PAUSE"
    START = "START"

class StreamServer:
    def __init__(self, host="0.0.0.0", port=8000):
        self.host = host
        self.port = port
        self.clients = set()
        self.camera_websocket = None
        self.stream_state = StreamStates.PAUSE
        self.current_config = None

    def get_host(self):
        return self.host if self.host != "0.0.0.0" else socket.gethostbyname(socket.gethostname())

    async def handle_connection(self, websocket, path):
        print(f"Connection from {websocket.remote_address} at {path}")
        if path == "/camera":
            await self.handle_camera_connection(websocket)
        elif path == "/":
            await self.handle_client_connection(websocket)

    async def handle_camera_connection(self, websocket):
        await self.disconnect_existing_camera()
        self.camera_websocket = websocket
        await self.start_stream_if_needed()

        try:
            async for message in websocket:
                await self.on_camera_message(message)
        except websockets.ConnectionClosedError:
            print(f"Camera disconnected from {websocket.remote_address}")
        finally:
            await self.cleanup_camera_connection()

    async def handle_client_connection(self, websocket):
        self.clients.add(websocket)
        await self.start_stream_if_needed()
        await self.send_current_config(websocket)

        try:
            async for message in websocket:
                await self.on_client_message(message, websocket)
        except websockets.ConnectionClosedError:
            print(f"Client disconnected from {websocket.remote_address}")
        finally:
            await self.cleanup_client_connection(websocket)

    async def on_camera_message(self, message):
        if isinstance(message, bytes) and self.should_send_image():
            await self.broadcast_image(message)
        elif isinstance(message, str):
            if message.startswith(Messages.CONFIG_PREFIX):
                self.current_config = message
                await self.broadcast_config()

    async def on_client_message(self, message, websocket):
        if message == Messages.END_CONNECTION:
            await websocket.close()
        elif message.startswith(Messages.COMMAND_PREFIX):
            if self.camera_websocket:
                await self.broadcast_command(message)
                await self.camera_websocket.send(message)

    async def send_current_config(self, websocket):
        if self.current_config:
            await websocket.send(self.current_config)

    async def broadcast_config(self):
        await asyncio.gather(*[client.send(self.current_config) for client in self.clients])

    async def broadcast_command(self, command):
        await asyncio.gather(*[client.send(command) for client in self.clients])

    async def broadcast_image(self, image_data):
        image_final = self.process_image(image_data)
        await asyncio.gather(*[client.send(image_final) for client in self.clients])

    def process_image(self, image_data):
        # We can process the image here if needed, like using OpenCV with face recognition
        return image_data

    async def disconnect_existing_camera(self):
        if self.camera_websocket:
            await self.camera_websocket.close()
            await self.set_stream_state(StreamStates.PAUSE)

    async def cleanup_camera_connection(self):
        self.camera_websocket = None
        await self.update_stream_state()

    async def cleanup_client_connection(self, websocket):
        self.clients.discard(websocket)
        await websocket.close()
        await self.update_stream_state()

    async def start_stream_if_needed(self):
        if self.should_send_image():
            await self.set_stream_state(StreamStates.START)

    def should_send_image(self):
        return bool(self.clients) and self.camera_websocket is not None

    async def set_stream_state(self, state):
        if state != self.stream_state:
            self.stream_state = state
            await self.broadcast_stream_state()

    async def broadcast_stream_state(self):
        state_message = f"{Messages.STREAM_PREFIX}{self.stream_state}"
        await asyncio.gather(*[client.send(state_message) for client in self.clients])
        if self.camera_websocket:
            await self.camera_websocket.send(state_message)

    async def update_stream_state(self):
        if self.should_send_image():
            await self.set_stream_state(StreamStates.START)
        else:
            await self.set_stream_state(StreamStates.PAUSE)

    async def start_server(self):
        async with websockets.serve(self.handle_connection, self.host, self.port):
            print(f"Server started at ws://{self.get_host()}:{self.port}")
            await asyncio.Future()  # Keep the server running

    def run(self):
        asyncio.run(self.start_server())

if __name__ == "__main__":
    print("StreamServer")
    stream_server = StreamServer(host="0.0.0.0", port=8000)
    stream_server.run()
