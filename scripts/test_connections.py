import asyncio
import websockets


# Define an async function to connect to the server
async def connect_to_server(server_url, counter):
    res = "Server nro {} ".format(counter)

    try:
        # Connect to the WebSocket server
        async with websockets.connect(server_url) as websocket:
            print(res + "is up!")
    except:
        print(res + "is not up!")


def main():
    with open("./url", "r") as f:
        counter = 1

        # Connect to all addresses in file line by line
        for line in f:
            asyncio.run(connect_to_server(line, counter))
            counter += 1


if __name__ == "__main__":
    main()

