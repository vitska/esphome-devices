#pragma once

// Helper for the periodic "which access points can this device hear" scan.
//
// ESPHome's own scan entry point (WiFiComponent::start_scanning) drives the STA
// connect state machine: on an AP-only config it would finish the scan, find no
// matching network and then retry connection phases against an empty network
// list. This helper only asks the ESP-IDF driver for a scan - the wifi
// component's WIFI_EVENT_SCAN_DONE handler still fills its scan-result list and
// notifies the wifi_info: scan_results listener, so the automation and the
// entity work exactly as usual while the state machine is left alone.

#include <esp_wifi.h>

namespace wifi_scan {

// Starts a non-blocking scan (results arrive through the driver's event).
// Returns false when the radio is not running, so an interval can skip a cycle
// instead of failing noisily.
inline bool start() {
  wifi_mode_t mode;
  if (esp_wifi_get_mode(&mode) != ESP_OK)
    return false;  // driver not initialised
  if (mode == WIFI_MODE_NULL)
    return false;  // radio stopped (fallback AP currently not needed)

  if (mode == WIFI_MODE_AP) {
    // Scanning needs a station interface; AP+STA keeps the fallback AP up.
    if (esp_wifi_set_mode(WIFI_MODE_APSTA) != ESP_OK)
      return false;
  }

  wifi_scan_config_t cfg{};
  cfg.ssid = nullptr;
  cfg.bssid = nullptr;
  cfg.channel = 0;
  cfg.show_hidden = true;
  cfg.scan_type = WIFI_SCAN_TYPE_ACTIVE;
  cfg.scan_time.active.min = 100;  // ms per channel
  cfg.scan_time.active.max = 300;
  return esp_wifi_scan_start(&cfg, false) == ESP_OK;
}

}  // namespace wifi_scan
