import asyncio
import sys
import yaml
import aioesphomeapi

async def main():
    with open(r"C:\Temp\personal\esphome-devices\secrets.yaml", "r") as f:
        secrets = yaml.safe_load(f)
    key = secrets.get("api.rotary-sh1116", "")

    cli = aioesphomeapi.APIClient(
        address="192.168.3.201",
        port=6053,
        password="",
        noise_psk=key,
        client_info="log_viewer",
    )

    try:
        await cli.connect(login=True)
        print("Connected!", flush=True)
    except Exception as e:
        print(f"Connection error: {e}", flush=True)
        return

    def on_log(msg):
        try:
            print(msg.message, flush=True)
        except Exception:
            print(repr(msg.message), flush=True)

    await cli.subscribe_logs(on_log, log_level=aioesphomeapi.LogLevel.LOG_LEVEL_VERY_VERBOSE)
    await asyncio.sleep(35)
    await cli.disconnect()

asyncio.run(main())
