# Load Custom Bitstream to OCT FPGAs
1. You will to generate a `*.mcs file` for this.

1. Make sure that your FPGA is in factory mode (golden image). 
    ```
    source /opt/xilinx/xrt/setup.sh
    xbmgmt examine
    ```
    It should show something like with the golden image `xilinx_u280_GOLDEN_8`: 
    ```
    Devices present
    BDF             :  Shell                 Platform UUID  Device ID  Device Ready*
    ----------------------------------------------------------------------------------
    [0000:3b:00.0]  :  xilinx_u280_GOLDEN_8  n/a            n/a        No
    ```
    
    If not, use the `config-fpga reset private_key.pem` to reset the FPGA to golden image.

    The `private_key.pem` file is the private key used to access the head node that performs the reset. You can generate the public-private key pair by running the following commands:

    ```
    openssl genpkey -algorithm RSA -out private_key.pem -pkeyopt rsa_keygen_bits:2048
    ```
    ```
    openssl rsa -pubout -in private_key.pem -out public_key.pem
    ``` 
    After generating the keypair, share your public key with us. This will allow you to use your private key to run `config-fpga`.
    
    It may take a few minutes to finish. After finish, it will show something like: 
    ```
    zhhan@pc164 ~ config-fpga reset private_key.pem
    Trying to reset the FPGA...
    Successfully flashed the FPGA with golden image.
    Trying to reset the PCIe bus...
    PCIe device 3a:00.0 has been reset.
    ```
     
    Use the `xbmgmt examine` to double check again. 

1. On OCT, we use PCIe programming to load the bitstreams. On OCT the FPGA device is generally located at `0000:3b:00.0`.

    For PCIe programming (on OCT nodes): we utilize Xilinx xbflash2 tool to program the FPGA. 
    ```
    sudo xbflash2 program --spi --image <path to the mcs> -d 3b:00.0 --bar 2
    ```
    It should return similar to:

    ```
    zhhan@pc164 ~ sudo xbflash2 program --spi --image bits/opennic_0x02150250.mcs -d 3b:00.0 --bar 2
    Preparing to program flash on device: 3b:00.0
    Are you sure you wish to proceed? [Y/n]: Y
    Successfully opened /dev/xfpga/flash.m15104.0
    flashing via QSPI driver
    Bitstream guard installed on flash @0x1002000
    Extracting bitstream from MCS data:
    .......................................
    Extracted 40523332 bytes from bitstream @0x1002000
    Writing bitstream to flash 0:
    .......................................
    Bitstream guard removed from flash
    ****************************************************
    Cold reboot machine to load the new image on device.
    ****************************************************
    ```
1. Instead of cold reboot, we can boot the FPGA from configuration memory to install the bitstream. Run 
   ```
   config-fpga boot private_key.pem
   ```
   It should show:
   ```
   zhhan@pc164 ~ config-fpga boot private_key.pem
   Trying to boot the FPGA...
   Successfully pulled the FPGA configuration from flash.
   Trying to reset the PCIe bus...
   PCIe device 3a:00.0 has been reset.
   ```
1. Use `lspci | grep 3b:00.0` to check your new FPGA device.
