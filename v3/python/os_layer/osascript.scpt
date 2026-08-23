if application "appName" is running then
    tell application "appName"
        activate
        tell application "System Events" to keystroke "letter" using {modifiers}
    end tell
end if