## Release Notes 

The At-RTOS latest version in the production branch is ![GitHub Release](https://img.shields.io/github/v/release/At-EC/At-RTOS) and the detail release notes is shown as the following table:

| Version |  Date      | Description  |
| ------- | -----------|--------------|
| v1.2.0  | 02/25/2024 | Add function os_kernal_is_running to allow user to check if At-RTOS is running. |
|         |            | Allocate a bit for pass information postcode, The maximum instance number up to 31. |
|         |            | Add an option into sem_init to allow user to reduce useless thread wakeup calling. |
|         |            | New interface is able to dump the current running thread id. |
|         |            | Enable discord At-RTOS service into github for contributors. |
|         |            | Add the release note file to record the code changes history. |
|         |            | Enable clang-format in the kernal source code. |
|         |            | Modify the README.md file content.|
| v1.1.0  | 02/16/2024 | Add the AtOS extern symbol for convenience use. |
|         |            | Add the license content into script and file header. |
| v1.0.0  | 02/15/2024 | Welcome to At-RTOS. This is the first release. A stable RTOS feature was implemented in the kernal system, Pls enjoy it (: |
