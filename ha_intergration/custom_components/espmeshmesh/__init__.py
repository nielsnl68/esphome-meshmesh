"""The espMeshMesh cooordinator integration."""

from __future__ import annotations

import logging
from datetime import timedelta
from typing import Any, Callable

import homeassistant.helpers.config_validation as cv
import serial
import voluptuous as vol
from espMeshMeshHub import espMeshMeshHubSerial
from homeassistant.config_entries import ConfigEntry
from homeassistant.const import (
    CONF_DEVICE_ID,
    Platform,
)
from homeassistant.core import (
    CALLBACK_TYPE,
    HomeAssistant,
    ServiceCall,
    SupportsResponse,
    callback,
)
from homeassistant.exceptions import ConfigEntryNotReady
from homeassistant.helpers import entity_registry
from homeassistant.helpers.entity import DeviceInfo
from homeassistant.helpers.update_coordinator import DataUpdateCoordinator, UpdateFailed

from .const import (
    CONF_BAUD_RATE,
    CONF_DEFAULT_INTERVAL,
    CONF_INTERVAL,
    CONF_SERIAL_PORT,
    DOMAIN,
)

_LOGGER = logging.getLogger(__name__)

PLATFORMS: list[Platform] = [
#    Platform.MEDIA_PLAYER,
#    Platform.SENSOR,
#    Platform.SWITCH,
#    Platform.SELECT,
#    Platform.NUMBER,
]

CONF_SERVICE_COMMAND = "command"
CONF_SERVICE_ACTION = "action"

SERVICE_SEND_SCHEMA = vol.Schema(
    {
        vol.Required(CONF_DEVICE_ID): cv.string,
        vol.Required(CONF_SERVICE_COMMAND): cv.string,
        vol.Optional(CONF_SERVICE_ACTION): cv.string,
    }
)
SERVICE_SEND_RAW_SCHEMA = vol.Schema(
    {
        vol.Required(CONF_DEVICE_ID): cv.string,
        vol.Required(CONF_SERVICE_COMMAND): cv.string,
    }
)


class espMeshMeshHubCoordinator(DataUpdateCoordinator):
    """espMeshMesh cooordinator Data Update Coordinator."""

    unique_id = None
    model = None
    device_info: DeviceInfo = None

    def __init__(self, hass, coordinator: espMeshMeshHubSerial):
        """Initialize espMeshMesh cooordinator Data Update Coordinator."""
        super().__init__(
            hass,
            _LOGGER,
            # Name of the data. For logging purposes.
            name=__name__,
        )

        self.coordinator = coordinator
        self.coordinator.add_listener(self._listener)

        self.unique_id = self.coordinator.unique_id

        self.device_info = DeviceInfo(
            identifiers={(DOMAIN, self.unique_id)},
            name="espMeshMesh",
            manufacturer="espHome",
        )

    @callback
    def _listener(self, command: str, data):
        self.async_set_updated_data({command: data})

    # async def async_connect(self):
    #     try:
    #         if not await self.cooordinator.connect():
    #             raise ConfigEntryNotReady(
    #                 f"Unable to connect to espMeshMesh cooordinator on {self.cooordinator.connection}"
    #             )
    #     except TimeoutError as ex:
    #         raise ConfigEntryNotReady(
    #             f"Unable to connect to espMeshMesh cooordinator on {self.cooordinator.connection}", ex
    #         )
    #
    #     _LOGGER.debug("Connected to espMeshMesh cooordinator on %s", self.cooordinator.connection)
    #
    #     self.unique_id = self.cooordinator.unique_id
    #
    #     self.device_info = DeviceInfo(
    #         identifiers={(DOMAIN, self.unique_id)},
    #         name=f"espMeshMesh",
    #         manufacturer="espMeshMesh",
    #     )

    async def async_disconnect(self):
        await self.cooordinator.disconnect()
        _LOGGER.debug(
            "Disconnected from espMeshMesh cooordinator on %s", self.cooordinator.connection
        )

    @callback
    def async_add_listener(
        self, update_callback: CALLBACK_TYPE, context: Any = None
    ) -> Callable[[], None]:
        self.cooordinator.add_listener(command=context)

        return super().async_add_listener(update_callback, context)

    def supports_command(self, command: str):
        return self.cooordinator.supports_command(command)

    async def async_send_command(self, command: str, action: str | None = None):
        if action:
            return await self.cooordinator.send_command(command, action)
        return await self.cooordinator.send_command(command)

    async def async_send_raw_command(self, command: str):
        return await self.cooordinator.send_raw_command(command)



async def async_setup_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    """Set up espMeshMesh cooordinator from a config entry."""
    cooordinator = None

    interval = entry.options.get(CONF_INTERVAL, CONF_DEFAULT_INTERVAL)

    serial_port = entry.data[CONF_SERIAL_PORT]
    baud_rate = entry.data[CONF_BAUD_RATE]

    cooordinator = espMeshMeshHubSerial(serial_port, baud_rate)


    # Open the connection.
    if not await cooordinator.connect(interval=interval):
        raise ConfigEntryNotReady(f"Unable to connect to device {cooordinator.unique_id}")

    _LOGGER.info("Device %s is available", cooordinator.unique_id)

    coordinator = espMeshMeshHubCoordinator(hass, cooordinator)

    hass.data.setdefault(DOMAIN, {})[entry.entry_id] = coordinator

    await hass.config_entries.async_forward_entry_setups(entry, PLATFORMS)

    entry.async_on_unload(entry.add_update_listener(update_listener))

    async def async_handle_send(call: ServiceCall):
        """Handle the send service call."""
        command: str = call.data.get(CONF_SERVICE_COMMAND)
        action: str = call.data.get(CONF_SERVICE_ACTION)

        response = await coordinator.cooordinator.send_command(command, action, False)

        return {"response": response}

    async def async_handle_send_raw(call: ServiceCall):
        """Handle the send_raw service call."""
        command: str = call.data.get(CONF_SERVICE_COMMAND)

        response = await coordinator.async_send_raw_command(command)

        return {"response": response}

    hass.services.async_register(
        DOMAIN,
        "send",
        async_handle_send,
        schema=SERVICE_SEND_SCHEMA,
        supports_response=SupportsResponse.ONLY,
    )

    hass.services.async_register(
        DOMAIN,
        "send_raw",
        async_handle_send_raw,
        schema=SERVICE_SEND_RAW_SCHEMA,
        supports_response=SupportsResponse.ONLY,
    )

    return True


async def async_unload_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    """Unload a config entry."""
    coordinator: espMeshMeshHubCoordinator = hass.data[DOMAIN][entry.entry_id]
    await coordinator.async_disconnect()

    if unload_ok := await hass.config_entries.async_unload_platforms(entry, PLATFORMS):
        hass.data[DOMAIN].pop(entry.entry_id)

    return unload_ok


async def update_listener(hass: HomeAssistant, entry: ConfigEntry) -> None:
    """Handle options update."""
    hass.config_entries.async_schedule_reload(entry.entry_id)
