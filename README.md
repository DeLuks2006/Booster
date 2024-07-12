<h1 align=center>Booster</h1>
<p align=center>Simple driver used to set a specific thread priority</p>

> [!NOTE]
> (this driver was created while reading/following the "Windows Kernel Programming" book by Pavel Yosifovich)

## Usage:
1.  Setup: Create Windows service:
   ```shell
   # make sure to execute this in cmd
   # setup
   sc create Booster type= kernel binpath= C:\Users\Path\To\Your\Driver.sys
   ```

2. Start Service:
  ```shell
  sc start Booster
  ```
3. Use client (Boost.exe): 
  ```shell
  .\Boost.exe <TID> <PRIORITY>
  # note priority can be from 0 - 31
  ```
4. Stop Service:
```shell
sc stop Booster
```
