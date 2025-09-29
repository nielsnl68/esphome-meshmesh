import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import (
    CONF_CHANNEL,
    CONF_ID,
    CONF_PASSWORD,
    CONF_TYPE
)
from esphome.core import CORE
from esphome.components import uart

CODEOWNERS = ["@persuader72"]
AUTO_LOAD = ["network", "md5", "uart"]


meshmesh_ns = cg.esphome_ns.namespace("meshmesh")
MeshmeshComponent = meshmesh_ns.class_("MeshmeshComponent", cg.Component)
MeshmeshCoordinator = meshmesh_ns.class_(
    "MeshmeshCoordinator", MeshmeshComponent, uart.UARTDevice
)
MeshmeshNode = meshmesh_ns.class_("MeshmeshNode", MeshmeshComponent, uart.UARTDevice)


CONF_BONDING_MODE = "bonding_mode"
CONF_COORDINATOR = "coordinator"
CONF_NODE = "node"


def validate_channel(value):
    value = cv.positive_int(value)
    if value < 1:
        raise cv.Invalid("Minimum WiFi channel is 1")
    if value > 13:
        raise cv.Invalid("Maximum WiFi channel is 13")
    return value


BASIC_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_CHANNEL): validate_channel,
        cv.Required(CONF_PASSWORD): cv.string,
        cv.Optional(CONF_BONDING_MODE, default=False): cv.boolean,
    }
).extend(cv.COMPONENT_SCHEMA)


CONFIG_SCHEMA = cv.typed_schema(
    {
        CONF_NODE: BASIC_SCHEMA.extend(
            {
                cv.GenerateID(): cv.declare_id(MeshmeshNode),
            }
        ),
        CONF_COORDINATOR: BASIC_SCHEMA.extend(
            {
                cv.GenerateID(): cv.declare_id(MeshmeshCoordinator),
            }
        ).extend(uart.UART_DEVICE_SCHEMA),
    },
    default_type=CONF_NODE,
)


async def to_code(config):
    cg.add_define("USE_MESH_MESH")
    cg.add_define("USE_POLITE_BROADCAST_PROTOCOL")
    cg.add_define("USE_MULTIPATH_PROTOCOL")
    cg.add_define("USE_CONNECTED_PROTOCOL")
    if config.get(CONF_BONDING_MODE, False):
        cg.add_define("USE_BONDING_MODE")

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_channel(config[CONF_CHANNEL]))
    cg.add(var.setAesPassword(config[CONF_PASSWORD]))
    if config[CONF_TYPE] == CONF_COORDINATOR:
        await uart.register_uart_device(var, config)

    if CORE.is_esp8266:
        cg.add_build_flag("-Wl,-wrap=ppEnqueueRxq")
        cg.add_library("ESP8266WiFi", None)

    cg.add_library(
        name="ESPMeshMesh",
        version="1.0.9",
        repository="https://github.com/EspMeshMesh/espmeshmesh#sockets",
    )
