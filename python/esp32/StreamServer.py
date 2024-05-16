import asyncio
import websockets
import socket
import cv2
import numpy as np

class CamStates:
    PAUSE = "[PAUSE]"
    START = "[START]"

class StreamServer:
    def __init__(self, host="0.0.0.0", port=8000):
        self.host = host
        self.port = port
        self.connected_clients = set()
        self.esp32cam = None
        self.stream = CamStates.PAUSE

    def get_host(self):
        return self.host if self.host != "0.0.0.0" else socket.gethostbyname(socket.gethostname())

    async def handle_connection(self, websocket, path):
        print(f"Connection from {websocket.remote_address} at {path}")
        if path == "/camera":
            await self.handle_camera_connection(websocket)
        elif path == "/":
            await self.handle_client_connection(websocket)

    async def handle_camera_connection(self, websocket):
        if self.esp32cam is not None:
            await self.esp32cam.close()
            await self.set_stream(CamStates.PAUSE)
        self.esp32cam = websocket
        if self.must_send_image():
            await self.set_stream(CamStates.START)
        try:
            async for image_data in websocket:
                if image_data:
                    await self.send_image_broadcast(image_data)
        except websockets.ConnectionClosedError:
            pass
        finally:
            print(f"Camera disconnected from {websocket.remote_address}")
            self.esp32cam = None
            if not self.must_send_image():
                await self.set_stream(CamStates.PAUSE)
            else:
                await self.send_state_broadcast()

    async def handle_client_connection(self, websocket):
        self.connected_clients.add(websocket)
        if self.must_send_image():
            await self.set_stream(CamStates.START)
        try:
            async for message in websocket:
                if message == "[END_CONNECTION]":
                    break
                elif message.startswith("[COMMAND]") and self.esp32cam is not None:
                    await self.send_command_broadcast(message)
        except websockets.ConnectionClosedError:
            pass
        finally:
            if websocket in self.connected_clients:
                self.connected_clients.remove(websocket)
            await websocket.close()
            if not self.must_send_image():
                await self.set_stream(CamStates.PAUSE)

    def must_send_image(self):
        return bool(self.connected_clients) and self.esp32cam is not None

    async def set_stream(self, value):
        if value != self.stream:
            self.stream = value
            await self.send_state_broadcast()

    async def send_state_broadcast(self):
        state = f"[STREAM]{self.stream}"
        await asyncio.gather(*[client.send(state) for client in self.connected_clients])
        if self.esp32cam is not None:
            await self.esp32cam.send(state)

    async def send_command_broadcast(self, command):
        if self.must_send_image():
            await asyncio.gather(*[client.send(command) for client in [self.esp32cam, *self.connected_clients]])

    async def send_image_broadcast(self, image_data):
        if self.must_send_image():
            nparr = np.frombuffer(image_data, np.uint8)
            image = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
            image_flipped = cv2.flip(image, 1)
            ret, image_data_flipped = cv2.imencode('.jpg', image_flipped)
            await asyncio.gather(*[client.send(image_data_flipped.tobytes()) for client in self.connected_clients])

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
