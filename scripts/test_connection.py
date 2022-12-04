import asyncio
import websockets


# Define an async function to connect to the server
async def connect_to_server(server_url):
    try:
        # Connect to the WebSocket server
        async with websockets.connect(server_url) as websocket:
            # If the connection is successful, print a message
            print("Server is up!")
    except:
        # If the connection fails, print an error message
        print("Server is not up!")


def main():
    with open("./url") as f:
        server_url = f.readlines()
    asyncio.run(connect_to_server(server_url))


if __name__ == "__main__":
    main()

