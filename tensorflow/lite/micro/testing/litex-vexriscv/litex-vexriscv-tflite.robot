*** Settings ***
Suite Setup                   Setup
Suite Teardown                Teardown
Test Setup                    Reset Emulation
Resource                      /opt/renode/tests/renode-keywords.robot


*** Test Cases ***
Run TF Model   
    ${BIN} =                  Get Environment Variable    BIN
    ${SCRIPT_DIR} =           Get Environment Variable    SCRIPT_DIR
    ${EXPECTED} =             Get Environment Variable    EXPECTED

    Execute Command           using sysbus

    Execute Command           include @${SCRIPT_DIR}/LiteX_I2C_Zephyr.cs

    Execute Command           mach create
    Execute Command           machine LoadPlatformDescription @${SCRIPT_DIR}/litex-vexriscv-tflite.repl

    Execute Command           showAnalyzer uart Antmicro.Renode.Analyzers.LoggingUartAnalyzer

    Execute Command           $bin = @${BIN}
    Execute Command           sysbus LoadELF $bin

    Create Terminal Tester    sysbus.uart  timeout=30

    Start Emulation

    Wait For Line On Uart     Booting Zephyr OS

    Wait For Line On Uart     ${EXPECTED}
