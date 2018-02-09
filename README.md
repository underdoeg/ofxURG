# ofxURG
Wrapper to use Hokuyo URG-04LX sensors in openframeworks

Tested on Arch Linux and Ubuntu 16.04 LTS. Should work with all linux distributions.

## Connecting to `/dev/ACM0` not working?
The URG-04LX device does not need any special drivers for Linux, but because it uses the ACM protocol, setup may not be completely straightforward.

If you can see the device is connected but "can't be opened", try to add your user to the `dialout` group:
```
sudo usermod -a -G dialout $USER
```
After this, you **must** log out and log in again before retrying.