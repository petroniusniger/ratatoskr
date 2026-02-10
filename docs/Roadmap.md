# Roadmap for `ratatoskr` app
(pan, v4, 10/02/2026)


## Generalities

- [x] Contact Michael Zanetti to determine appropriate license
- [x] Update ADR-005 with final display issue resolution (`Window` object)
- [ ] Comment code where needed
- [ ] Synthetise ADRs where possible

## Enhancements

### Main app

- [ ] Add confirmation pop-up before deleting received file from ItemList
- [ ] Add confirmation pop-up before accepting incoming file over BT
- [ ] Get rid of 'unconfined' template in AA profile (may require platform 
  changes)
- [ ] Code cleanup: remove objects that are not needed (BluetoothManager?,
  DeviceDiscovery?, OBEXTransferService?)
- [ ] App logs its version on startup

### SharePlugin

- [x] Rename "Bluetooth" desktop icon to "BT Plugin"
- [ ] Find way of hiding plugin icon on desktop -- may require to redesign
  plugin as Headless ContentHub peer (see trace from Sprint #006)
- [ ] Get rid of 'unconfined' template in AA profile (may require platform 
  changes)
- [ ] if at all possible, the list should also allow to identify the devices
  that are already paired and/or connected over Bluetooth
- [ ] a user preference dialog could be added to control the amount of 
  filtering used to populate the device list:
  - filter out devices whose name is their MAC address
  - filter out devices that are not paired
- [ ] SharePlugin logs its version on startup

## Bugs

### Main app

### SharePlugin

- [x] Plugin crashes and causes phone to restart
- [ ] Plugin has no icon -- should use `ratatoskr/shareplugin/icon.svg` (BT 
  logo)
- [x] the device list at present only contains MAC addresses -- it should also 
  show the device name
- [ ] display warning pop-up if started from AppDrawer i/o "Share" icon
