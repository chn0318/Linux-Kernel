# Linux-Kernel

#### Project: Android Kernel Scheduler

- Implemented a new weighted Round-Robin process scheduling method for Linux 
- Deployed the new scheduler to Android phones.

#### Project: Benchmark

- In kernel mode, write a kernel module that can obtain the actual runtime and memory read/write of a program; 
- In user mode, write a program that periodically tracks the CPU utilization and memory read/write of a program through the kernel module, and compares and analyzes the program to see if it is computationally intensive or memory intensive.

#### Project: File system

Using the `/fs/romfs` file system source code in the Linux kernel as the base, modify and compile to generate the `romfs.ko` module, and implement the following functions:

- `romfs.ko` accepts 3 parameters： `hided_file_name` , `encrypted_file_name` and `exec_file_name`

  - hided_file_name=xxx : File path to be hidden
  - encrypted_file_name=xxx : File path to be encrypted
  - exec_file_name=xxx ： File path with executable permission

  Through ` insmod romfs Ko `installs the modified `romfs` module, uses ` genromfs` to generate an image file in the format of `romfs`, and uses the' mount 'command to mount the image file and verify whether the modification is successful.

#### Project: memory space

- Implement Kernel Modules: The user can communicate with the module through the proc file system to read and write the memory space of any process.
- Implement Kernel Modules: Create a proc file, set up corresponding mapping for user when using `mmap` system call.

#### Project: Process Scheduler

Modify the Linux source code to add a record of the number of dispatches per process

**Notice:** please refer to report.pdf in each file folder for more detail

