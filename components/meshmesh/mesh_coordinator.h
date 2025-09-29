#pragma once
#ifdef USE_MESH_COORDINATOR

#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/components/meshmesh/meshmesh.h"

namespace esphome::meshmesh {

enum RecvState {
  WAIT_START,
  WAIT_DATA,
  WAIT_ESCAPE,
  WAIT_CRC16_1,
  WAIT_CRC16_2,
};
enum SpecialByteCodes {
  CODE_DATA_START = 0xFE,
  CODE_DATA_START_CRC16 = 0xFD,
  CODE_DATA_END = 0xEF,
  CODE_DATA_ESCAPE = 0xEA
};

class MeshmeshCoordinator : public MeshmeshComponent, public uart::UARTDevice {
 public:
  void setup() override;
  void dump_config() override;
  void loop() override;

 protected:
  int8_t recv_from_mesh(uint8_t *buf, uint16_t size, uint32_t address) override;
  int8_t send_to_mesh(uint8_t *buf, uint16_t size, uint32_t address) override;

  void read_from_uart_(uint8_t byte);
  int8_t write_to_uart_(uint8_t *buf, uint16_t size, uint32_t address);

 private:
  std::vector<uint8_t> buffer_;

  uint16_t computed_crc16_{};
  uint16_t received_crc16_{};
  RecvState receive_state_{WAIT_START};

  uint8_t received_from_id_[4];
  uint8_t received_path_[32];
  uint8_t received_path_size_ = 0;
};

}  // namespace esphome::meshmesh
#endif
