# Home Assistant integration for espMeshMesh

## Introduction

Home Assistant integration to control the espMeshMesh network Nodes build around ESPHome over the serial.

## Features

* Connects to the espMeshMesh network via a serial Coordintor over serial or network interface
* Sending commands to specific nodes
* Reading the node status
* Uses asynchronous IO

## Protocol

espMeshMesh uses the 802.11 protocol to connect different device to allow the esphome API to report back to Home-Assistant. This
integration supports not only the API but support also an option to update node via the commoin protocol.

## Hardware

### Serial port

To allow this integration to work you need to connect an ESP32 via USB to your own Home-Assistant. running esphome with *MeshMesh:* setup as serial bridge. [firmware](https://github.com/EspMeshMesh/esphome-meshmesh/blob/main/coordinator-esp32-wroom32.yaml)

## Adding a new espMeshMesh Node

- After restarting go to **Settings** then **Devices & Services**
- Select **+ Add integration** and type in *espMeshMesh Hub*
- Select the serial port or enter the path manually
- Enter the baud rate
- Select **Submit**

A new espMeshMesh integration and device will now be added to your
Integrations view.

## Actions

The integration supports actions so commands can be send which are (not yet) implemented.

`benqprojector.send` This action allows you to send commands with or withouth action to your BenQ
Projector. To get the current state of a setting use `?` as the action.

```
action: benqprojector.send
data:
  device_id: 1481637509cb0c89ea1582e195fe6370
  command: "pow"
  action: "?"
```

`benqprojector.send_raw` This action allows you to send any raw command to your BenQ Projector. The
command needs to include the `*` and `#` prefix and suffix.

```
action: benqprojector.send_raw
data:
  device_id: 1481637509cb0c89ea1582e195fe6370
  command: "*pow=?#"
```

## Contribution and appreciation

You can contribute to this integration, or show your appreciation, in the following ways.

### Contribute your language

If you would like to use this Home Assistant integration in your own language you can provide a
translation file as found in the `custom_components/espMeshMesh/translations` directory. Create a
pull request (preferred) or issue with the file for your language attached.

More on translating custom integrations can be found
[here](https://developers.home-assistant.io/docs/internationalization/custom_integration/).

### Star this integration

Help other Home Assistant users find this integration by starring the [GitHub page of this integration](https://github.com/EspMeshMesh/esphome-meshmesh).
Click **⭐ Star** on the top right of the GitHub page.

### Support the work

Do you enjoy using this Home Assistant integration? Then consider supporting the work using one of
the following platforms, your donation is greatly appreciated and keeps me motivated:

[![Github Sponsors][github-shield]][github]
[![PayPal][paypal-shield]][paypal]
[![BuyMeCoffee][buymecoffee-shield]][buymecoffee]
[![Patreon][patreon-shield]][patreon]
