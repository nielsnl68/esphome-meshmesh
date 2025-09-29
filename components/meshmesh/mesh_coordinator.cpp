#ifdef USE_MESH_COORDINATOR

#include "mesh_coordinator.h"
#include "commands.h"
#include <packetbuf.h>
#include <discovery.h>
#include <espmeshmesh.h>

#include "esphome/core/application.h"
#include "esphome/core/log.h"
#include "esphome/core/preferences.h"
#include "esphome/core/application.h"
#include "esphome/core/version.h"



static const char *TAG = "meshmesh_coordinator";

namespace esphome::meshcoord {

void MeshmeshCoordinator::setup() {
  this->parent_->addHandleFrameCb(std::bind(&MeshmeshCoordinator::handle_frame_, this, std::placeholders::_1,
                                            std::placeholders::_2, std::placeholders::_3));
}

void MeshmeshCoordinator::dump_config() { ESP_LOGCONFIG(TAG, "Meshmesh_coordinator"); }

void MeshmeshCoordinator::loop() {
  uint8_t data;
  size_t avail = this->available();
  while (avail > 0) {
    this->read_from_uart_(this->read());
  }
}

void MeshmeshCoordinator::read_from_uart_(uint8_t byte) {
  switch (this->receive_state_) {
    case WAIT_START:
      if (byte == CODE_DATA_START_CRC16) {
        this->computed_crc16_ = 0;
        this->receive_state_ = WAIT_DATA;
        this->buffer_.clear();
        this->buffer_position_ = 0;
      }
      break;
    case WAIT_DATA:
      if (byte == CODE_DATA_END) {
        this->receive_state_ = WAIT_CRC16_1;
      } else {
        this->computed_crc16_ = crc_ibm_byte(this->computed_crc16_, byte);
        if (byte == CODE_DATA_ESCAPE) {
          this->receive_state_ = WAIT_ESCAPE;
        } else {
          this->buffer_.push_back(byte);
        }
      }
      break;
    case WAIT_ESCAPE:
      if (byte == CODE_DATA_ESCAPE || byte == CODE_DATA_END || byte == CODE_DATA_START) {
        this->computed_crc16_ = crc_ibm_byte(this->computed_crc16_, byte);
        this->buffer_.push_back(byte);
      }
      this->receive_state_ = WAIT_DATA;
      break;
    case WAIT_CRC16_1:
      this->received_crc16_ = uint16_t(byte) << 8;
      this->receive_state_ = WAIT_CRC16_2;
      break;
    case WAIT_CRC16_2:
      this->received_crc16_ = this->received_crc16_ | uint16_t(byte);
      if (this->computed_crc16_ == this->received_crc16_) {
        this->write_to_mesh_(this->buffer_.data(), this->buffer_.size());
      } else {
        // handleFrame(mRecvBuffer, mRecvBufferPos, SRC_SERIAL, 0xFFFFFFFF);
        LIB_LOGE(TAG, "CRC16 mismatch %04X vs %04X", this->computed_crc16_, this->received_crc16_);
      }
      this->receive_state_ = WAIT_START;
      break;
  }
}

void MeshmeshCoordinator::write_to_uart_(const uint8_t *buff, uint16_t len, uint32_t address) {
  uint16_t i;
  uint16_t crc16 = 0;

  this->write(CODE_DATA_START_CRC16);

  for (i = 0; i < len; i++) {
    if (buff[i] == 0xEA || buff[i] == 0xEF || buff[i] == 0xFE) {
      this->write(CODE_DATA_ESCAPE);
      crc16 = crc_ibm_byte(crc16, CODE_DATA_ESCAPE);
    }
    this->write(buff[i]);
    crc16 = crc_ibm_byte(crc16, buff[i]);
  }
  this->write(CODE_DATA_END);
  this->write(crc16 >> 8);
  this->write(crc16 & 0xFF);
  this->flush()
}

int8_t MeshmeshCoordinator::write_to_mesh_(uint8_t *buf, uint16_t len, uint32_t from) {
  int8_t err = FRAME_NOT_HANDLED;

  switch (buf[0]) {
    case CMD_UART_ECHO_REQ:
      if (len > 1) {
        buf[0] = CMD_UART_ECHO_REP;
        this->write_to_uart(buf, len);
        err = 0;
      }
      break;

    case CMD_BROADCAST_SEND:  // 70 AABBCCDDEE...ZZ
      if (len > 1) {
        this->broadCastSendData(buf + 1, len - 1);
        err = 0;
      }
      break;
    case CMD_UNICAST_SEND:  // 72 0000XXYY AABBCCDDEE...ZZ
      if (len > 5) {
        LIB_LOGD(TAG, "CMD_UNICAST_SEND len %d", len);
        this->uniCastSendData(buf + 5, len - 5, uint32FromBuffer(buf + 1));
        err = 0;
      }
      break;
    case CMD_MULTIPATH_SEND:  // 76 AAAAAAAA BB CCCCCCCC........DDDDDDDD AABBCCDDEE...ZZ
      if (len > 5) {
        uint8_t pathlen = buf[5];

        if (len > 6 + pathlen * sizeof(uint32_t)) {
          size_t headersize = 6 + pathlen * sizeof(uint32_t) size_t payloadsize = len - headersize;
          uint32_t address = uint32FromBuffer(buf + 1);

          this->multipathSendData(buf + headersize, payloadsize, address, pathlen, buf + 6);
          err = 0;
        }
      }
      break;
    case CMD_POLITEBRD_SEND:
      if (len > 5) {
        this->mPoliteBroadcast->send(buf + 5, len - 5, uint32FromBuffer(buf + 1));
        err = 0;
      }
      break;
    case CMD_CONNPATH_REQUEST:
      if (len > 1) {
        err = mConnectedPath->receiveUartPacket(buf + 1, len - 1);
      }
      break;
  }
  return err;
}

}  // namespace esphome::meshcoord
#endif
