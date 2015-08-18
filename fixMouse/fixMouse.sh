#!/bin/bash

FALSE="0"
TRUE="1"
FOREGROUND="${FALSE}"

function Log() {
    if [ "${FOREGROUND}" = "${FALSE}" ]; then
        echo "${@}" | logger -t "${0}[${$}]" > /dev/null 2>&1 &
    else
        echo "${@}" >&2
    fi
}

#
#  Report errors to stderr
#
function Abort() {
    Log "${@}"
    exit 2
}

#
#  This is our SIGQUIT/TERM signal handler.  Try and cleanup
#  any stray files.
#
function Cleanup() {
    if [ "${MOUSEKEYFILE}" = "" ]; then
        echo "No temporary file configured"
        return
    fi
    rm -v -f -- "${MOUSEKEYFILE}"
}

#
#  Check for the commands we need.  We will be running in the
#  background, so we try to prevent errors reporting
#  to /dev/null
#
for COMMAND in input-kbd logger dbus-monitor grep mktemp \
               xinput sed sort
do
    which "${COMMAND}" > /dev/null 2>&1
    if [ ${?} -ne 0 ]; then
        Abort "Required command [${COMMAND}] not found"
    fi
done
Log "Startup"

#
#  We monitor DBUS for freedesktop adding interfaces
#
#  grep ^signal is so we only execute for signals.  dbus-monitor
#  gives additional information that we ignore.
#
#  grep --line-buffered is to make sure we get the info
#  immediately.
#
#  We </dev/null to make sure we detach from the tty.
#
dbus-monitor \
    --monitor 'interface=org.freedesktop.DBus.ObjectManager,member=InterfacesAdded' \
    < /dev/null \
    | grep --line-buffered '^signal' \
    | (
    #
    #  On QUIT, cleanup
    #
    unset MOUSEKEYFILE # in case we inherit this.
    trap Cleanup EXIT

    #
    #  input-kbd needs to read from a file, so make sure we create
    #  one.
    #
    MOUSEKEYFILE=`mktemp --tmpdir mousekeyfile.XXXXXX`
    if [ ${?} -ne 0 ]; then
        Abort "Could not create temp file"
    fi

    #
    #  Now we'll loop forever, reading a line.
    #
    while :; do   
        read THROW_AWAY
        echo "Bluetooth device was added"
        #
        #  xinput for 'ultrathin'
        #  it lists 
        #     ...
        #     Device Node (257): "/dev/input/event17"
        #     ...
        #  but we only want '17'
        #
        #  so we grep for device node
        #  and use sed to grab just the number.
        #
        DEV_NUMBERS=`xinput --list-props "Ultrathin Touch Mouse" \
                   | grep "Device Node" \
                   | sed -e 's_.*/dev/input/event\([0-9]\+\).*_\1_'`

        #
        #  Not all bluetooth devices are the ones we want, so skip
        #  ahead.  'sleep' to prevent errors causing us to
        #  eat 100% CPU
        #
        if [ "${DEV_NUMBERS}" = "" ]; then
            echo "No Logitech Ultrathin Mouse found"
            sleep 1
            continue
        fi
        
        #
        #  We might have more than one ultrathin mouse.
        #  fix them all.
        #
        for DEV_NUMBER in ${DEV_NUMBERS}; do
            echo "Saving mouse #${DEV_NUMBER} to ${MOUSEKEYFILE}"
            
            #
            #  Empty the mouse key file.  This was a securely
            #  made temp file, so we just use it.
            #  we shouldn't be root anyway.
            #
            echo > "${MOUSEKEYFILE}"
            
            #
            #  We want KEYs, not BTNs.  We only want the mouse
            #  to stop sending keyboard events.
            #
            #  sort -u to strip out duplicates.  Apparently this
            #  mouse has *duplicate* events in addition to
            #  garbage ones.
            #
            #  input-kbd has a bug with duplicates.
            #
            sudo -n input-kbd "${DEV_NUMBER}" \
                | grep '# KEY' \
                | sed -e 's/=.*/= KEY_UNKNOWN/' \
                | sort -u \
                > "${MOUSEKEYFILE}"
            echo "Fixing mouse #${DEV_NUMBER}"

            #
            #  Save the changed configuration.
            #
            sudo -n input-kbd -f "${MOUSEKEYFILE}" "${DEV_NUMBER}" \
                2> /dev/null
        done
        sleep 1
    #
    #  End of the while loop.  Everything gets logged.
    #
done 2>&1 ) | logger -t "${0}[${$}]" > /dev/null 2>&1 &

