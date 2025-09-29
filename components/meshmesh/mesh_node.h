#pragma once
#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"

#ifdef USE_ESP8266
extern "C" {
void IRAM_ATTR __wrap_ppEnqueueRxq(void *a);
void IRAM_ATTR __real_ppEnqueueRxq(void *);
}
#endif

namespace espmeshmesh {
class EspMeshMesh;
}

namespace esphome::meshmesh {


class MeshNode : public MeshmeshComponent {
 public:
  void dump_config() override;

 private:
  int8_t recv_from_mesh(uint8_t *buf, uint16_t size, uint32_t address) override;
};

}  // namespace esphome::meshmesh
