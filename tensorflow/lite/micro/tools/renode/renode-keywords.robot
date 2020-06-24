*** Settings ***
Library         String
Library         Process
Library         Collections
Library         OperatingSystem
Library         helper.py

*** Variables ***
${SERVER_REMOTE_DEBUG}    False
${SERVER_REMOTE_PORT}     12345
${SERVER_REMOTE_SUSPEND}  y
${SKIP_RUNNING_SERVER}    False
${CONFIGURATION}          Release
${PORT_NUMBER}            9999
${DIRECTORY}              ${CURDIR}/../bin
${BINARY_NAME}            Renode.exe
${HOTSPOT_ACTION}         None
${DISABLE_XWT}            False

*** Keywords ***
Setup
    ${SYSTEM}=          Evaluate    platform.system()    modules=platform

    ${CONFIGURATION}=  Set Variable If  not ${SKIP_RUNNING_SERVER} and ${SERVER_REMOTE_DEBUG}
    ...    Debug
    ...    ${CONFIGURATION}

    # without --hide-log the output buffers may get full and the program can hang
    # http://robotframework.org/robotframework/latest/libraries/Process.html#Standard%20output%20and%20error%20streams
    @{PARAMS}=           Create List  --robot-server-port  ${PORT_NUMBER}  --hide-log

    Run Keyword If        ${DISABLE_XWT}
    ...    Insert Into List  ${PARAMS}  0  --disable-xwt

    Run Keyword If       not ${SKIP_RUNNING_SERVER}
    ...   File Should Exist    ${DIRECTORY}/${BINARY_NAME}  msg=Robot Framework remote server binary not found (${DIRECTORY}/${BINARY_NAME}). Did you forget to build it in ${CONFIGURATION} configuration?

    # this handles starting on Linux/macOS using mono launcher
    Run Keyword If       not ${SKIP_RUNNING_SERVER} and not ${SERVER_REMOTE_DEBUG} and not '${SYSTEM}' == 'Windows'
    ...   Start Process  mono  ${BINARY_NAME}  @{PARAMS}  cwd=${DIRECTORY}

    # this handles starting on Windows without an explicit launcher
    # we use 'shell=true' to execute process from current working directory
    Run Keyword If       not ${SKIP_RUNNING_SERVER} and not ${SERVER_REMOTE_DEBUG} and '${SYSTEM}' == 'Windows'
    ...   Start Process  ${BINARY_NAME}  @{PARAMS}  cwd=${DIRECTORY}  shell=true

    Run Keyword If       not ${SKIP_RUNNING_SERVER} and ${SERVER_REMOTE_DEBUG} and not '${SYSTEM}' == 'Windows'
    ...   Start Process  mono
          ...            --debug
          ...            --debugger-agent\=transport\=dt_socket,address\=0.0.0.0:${SERVER_REMOTE_PORT},server\=y,suspend\=${SERVER_REMOTE_SUSPEND}
          ...            ${BINARY_NAME}  @{PARAMS}  cwd=${DIRECTORY}

    Run Keyword If       not ${SKIP_RUNNING_SERVER} and ${SERVER_REMOTE_DEBUG} and '${SYSTEM}' == 'Windows'
    ...    Fatal Error  Windows doesn't support server remote debug option.

    #The distinction between operating systems is because localhost is not universally understood on Linux and 127.0.0.1 is not always available on Windows.
    Run Keyword If       not '${SYSTEM}' == 'Windows'
    ...   Wait Until Keyword Succeeds  60s  1s
          ...   Import Library  Remote  http://127.0.0.1:${PORT_NUMBER}/
    Run Keyword If       '${SYSTEM}' == 'Windows'
    ...   Wait Until Keyword Succeeds  60s  1s
          ...   Import Library  Remote  http://localhost:${PORT_NUMBER}/

    Reset Emulation

Teardown
    Run Keyword Unless  ${SKIP_RUNNING_SERVER}
    ...   Stop Remote Server

    Run Keyword Unless  ${SKIP_RUNNING_SERVER}
    ...   Wait For Process

Hot Spot
    Handle Hot Spot  ${HOTSPOT_ACTION}

