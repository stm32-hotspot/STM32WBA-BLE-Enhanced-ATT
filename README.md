# STM32WBA-BLE-EATT

Demonstrate STM32WBA acting as BLE central and GATT client with BLE_App_EATT_central and GAP peripheral and GATT server with BLE_App_EATT_periph.

BLE_App_EATT_central application scans and connects to BLE_App_EATT_periph device.

Both devices support EATT feature.

For more information regarding BLE EATT feature, please visit BLE EATT Wiki documentation (https://wiki.st.com/stm32mcu/wiki/Connectivity:STM32WB-WBA_EATT).

## Setup
These applications are running on two **NUCLEO-WBA55CGA boards**. 
Applications are derived from BLE_DataThroughput_Client (BLE_App_EATT_central) and BLE_DataThroughput_Server (BLE_App_EATT_periph) applications.
Open a VT100 terminal on Central and Peripheral side (ST Link Com Port, @115200 bauds).

At startup, devices are initialized.
 - The peripheral device starts advertising.
 - B1 on central device starts scanning. Scan is stopped if a BLE_EATT_Server is detected.

## Application description
These applications are based on BLE_DataThroughput_Client and BLE_DataThroughput_Server from STM32CubeWBA package v1.4.1.  

Two STM32WBA nucleo boards are used, one central/client (flashed with BLE_App_EATT_central) and one peripheral/server (flashed with BLE_App_EATT_periph). 

The GATT server board support a Data transfer service with a transmission characteristic that supports notification.

The GATT client board support a Data transfer by writing without response to the GATT server board Reception characteristic.

The board flashed with BLE_App_EATT_periph is defined as GAP peripheral.

Open a VT100 terminal on Central and Peripheral side (ST Link Com Port, @115200 bauds).

At reset application initialization is done.

 - The peripheral device starts advertising.
 - Push B1 on central: the central device scans and automatically connects to the peripheral (use of FW_ID_EATT_PERIPH). 
 - After BLE connection:
 - Services and characteristics discovery
 - Pairing and bonding requested
 - L2cap COC connection (SPSM = 0x27) - 2 channels are created, it corresponds to 2 bearers. Two ACI_GATT_EATT_BEARER_EVENT events are received on server and on client.

On server side when connected:

 - B1 starts (blue LED is ON)/stops (blue LED is OFF) notification on both bearers.

On server side when idle:

 - B2 clear database

On client side when connected:

 - B1 starts (blue LED is ON)/stops (blue LED is OFF) write without response on both bearers.

On client side when idle:

 - B2 clear database

On client terminal receiving the current notification, the number of bytes per second on each bearer is displayed.

On server terminal receiving the current write without response, the number of bytes per second on each bearer is displayed.
  
In app_conf.h :

 - if #define CFG_TX_PHY    2 and #define CFG_RX_PHY    2, link is set to 2M at connection
 - if #define CFG_TX_PHY    1 and #define CFG_RX_PHY    1, link stays at 1M

 ## Troubleshooting

**Caution** : Issues and the pull-requests are **not supported** to submit problems or suggestions related to the software delivered in this repository. The STM32WBA-BLE-EATT example is being delivered as-is, and not necessarily supported by ST.

**For any other question** related to the product, the hardware performance or characteristics, the tools, the environment, you can submit it to the **ST Community** on the STM32 MCUs related [page](https://community.st.com/s/topic/0TO0X000000BSqSWAW/stm32-mcus).


