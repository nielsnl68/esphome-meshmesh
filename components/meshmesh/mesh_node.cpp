#include "mesh_node.h"
#include "commands.h"
#include <packetbuf.h>
#include <discovery.h>


#include "esphome/core/application.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"


static const char *TAG = "meshmesh.node";


namespace esphome::meshmesh {


void MeshNode::dump_config() {
  ESP_LOGCONFIG(TAG, "Meshmesh.node");
  MeshmeshComponent::dump_config()
}


int8_t MeshNode::recv_from_mesh(uint8_t *data, uint16_t size, uint32_t from) {
  int8u_t result = MeshmeshComponent::recv_from_mesh(data, size, from);
  if (result != FRAME_NOT_HANDLED) {
    return result;
  }
  switch (data[0]) {

    case CMD_NODE_TAG_REQ:
      if (size == 1) {
        uint8_t reply[33] = {0};
        reply[0] = CMD_NODE_TAG_REP;
        memcpy(reply + 1, this->preferences_.devicetag, 32);
        this->mesh_->commandReply(reply, 33);
        return HANDLE_UART_OK;
      }
      break;
    case CMD_NODE_TAG_SET_REQ:
      if (size > 1) {
        memcpy(this->preferences_.devicetag, data + 1, size - 1);
        this->preferences_.devicetag[size - 1] = 0;
        this->preferences_object.save(&this->preferences_);
        data[0] = CMD_NODE_TAG_SET_REP;
        this->mesh_->commandReply(data, 1);
        return HANDLE_UART_OK;
      }
      break;
    case CMD_CHANNEL_SET_REQ:
      if (size == 2) {
        uint8_t channel = data[1];
        if (channel < MAX_CHANNEL) {
          this->preferences_.channel = channel;
          this->preferences_object.save(&this->preferences_);
          data[0] = CMD_CHANNEL_SET_REP;
          this->mesh_->commandReply(data, 1);
          return HANDLE_UART_OK;
        }
      }
      break;
    case CMD_NODE_CONFIG_REQ:
      if (size == 1) {
        uint8_t reply[sizeof(MeshmeshSettings) + 1];
        reply[0] = CMD_NODE_CONFIG_REP;
        memcpy(reply + 1, &this->preferences_, sizeof(MeshmeshSettings));
        this->mesh_->commandReply(reply, sizeof(MeshmeshSettings) + 1);
        return HANDLE_UART_OK;
      }
      break;
    case CMD_LOG_DEST_REQ:
      if (size == 1) {
        uint8_t reply[5] = {0};
        reply[0] = CMD_LOG_DEST_REP;
        espmeshmesh::uint32toBuffer(reply + 1, this->preferences_.log_destination);
        this->mesh_->commandReply(reply, 5);
        return HANDLE_UART_OK;
      }
      break;
    case CMD_LOG_DEST_SET_REQ:
      if (size == 5) {
        this->preferences_.log_destination = espmeshmesh::uint32FromBuffer(data + 1);
        this->preferences_object.save(&this->preferences_);
        data[0] = CMD_LOG_DEST_SET_REP;
        this->mesh_->commandReply(data, 1);
        return HANDLE_UART_OK;
      }
      break;
      case CMD_GROUPS_REQ:
      if (size == 1) {
        uint8_t reply[5] = {0};
        reply[0] = CMD_GROUPS_REP;
        espmeshmesh::uint32toBuffer(reply + 1, this->preferences_.groups);
        this->mesh_->commandReply(reply, 5);
        return HANDLE_UART_OK;
      }
      break;
    case CMD_GROUPS_SET_REQ:
      if (size == 5) {
        this->preferences_.groups = espmeshmesh::uint32FromBuffer(data + 1);
        this->preferences_object.save(&this->preferences_);
        data[0] = CMD_GROUPS_SET_REP;
        this->mesh_->commandReply(data, 1);
        return HANDLE_UART_OK;
      }
      break;
    case CMD_FIRMWARE_REQ:
      if (size == 1) {
        size_t reply_size = 1 + strlen(ESPHOME_VERSION) + 1 + App.get_compilation_time().length() + 1;
        uint8_t *reply = new uint8_t[reply_size];
        reply[0] = CMD_FIRMWARE_REP;
        strcpy((char *) reply + 1, ESPHOME_VERSION);
        reply[1 + strlen(ESPHOME_VERSION)] = ' ';
        strcpy((char *) reply + 1 + strlen(ESPHOME_VERSION) + 1, App.get_compilation_time().c_str());
        reply[size - 1] = 0;
        this->mesh_->commandReply((const uint8_t *) reply, reply_size);
        delete[] reply;
        return HANDLE_UART_OK;
      }
      break;
    case CMD_REBOOT_REQ:
      if (size == 1) {
        data[0] = CMD_REBOOT_REP;
        this->reboot_requested_ = true;
        this->reboot_requested_time_ = millis();
        this->mesh_->commandReply(data, 1);
        return HANDLE_UART_OK;
      }
      break;
  }
  return FRAME_NOT_HANDLED;
}

}  // namespace meshmesh
