import asyncio
import sys
import yaml
import subprocess
import time
import aioesphomeapi

async def restart_device():
    with open(r"C:\Temp\personal\esphome-devices\secrets.yaml", "r") as f:
        secrets = yaml.safe_load(f)
    key = secrets.get("api.rotary-sh1116", "")

    cli = aioesphomeapi.APIClient(
        address="192.168.3.201",
        port=6053,
        password="",
        noise_psk=key,
        client_info="restarter",
    )

    try:
        await cli.connect(login=True)
        print("Connected, sending restart...", flush=True)
    except Exception as e:
        print(f"Connection error: {e}", flush=True)
        print("Trying OTA anyway...", flush=True)
        return

    try:
        # Find and press the restart button
        entities, services = await cli.list_entities_services()
        for entity in entities:
            if hasattr(entity, 'key') and hasattr(entity, 'name'):
                if 'restart' in entity.name.lower() or 'restart' in getattr(entity, 'object_id', '').lower():
                    print(f"Found restart button: {entity.name} (key={entity.key})", flush=True)
                    await cli.button_command(entity.key)
                    print("Restart command sent!", flush=True)
                    break
    except Exception as e:
        print(f"Restart error: {e}", flush=True)

    await cli.disconnect()

async def main():
    await restart_device()
    print("Waiting 8s for device to reboot...", flush=True)
    await asyncio.sleep(8)
    print("Starting OTA upload...", flush=True)

asyncio.run(main())
