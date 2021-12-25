// ==UserScript==
// @name         Big Red Mute Button - Google Meets Connector
// @namespace    http://tampermonkey.net/
// @version      1
// @description  try to take over the world!
// @author       Alice Fernandes
// @match        https://meet.google.com/**
// @grant        none
// ==/UserScript==

(function() {
    //TODO: Handle disconnects
    let selector = "[role='button'][data-is-muted]";
    let target = document.querySelector(selector);

    let mutationObserver = null;
    let writer = null;
    let port = null;
    let allowLoop = true;
    let streamWriter = null;


    //Check if target is avaliable
    function waitForMeets() {
        target = document.querySelector(selector);

        if (target == null) {
            window.setTimeout(waitForMeets, 1000);
        } else {
            let event = new CustomEvent("targetAquired");
            document.dispatchEvent(event);
        }



        //Update when changing the page;
        var observer = new MutationObserver(function (mutations) {
            mutations.forEach(function (mutation) {
                if(mutation.addedNodes.length > 0){
                    target = document.querySelector(selector);

                    if(mutationObserver != null){
                        mutationObserver.disconnect();
                        observeMutations(streamWriter);

                    }
                }
            });
        }
                                           );

        var config = {
            childList:true,
            attributes: true,
            attributeFilter: ['data-is-muted'],
        };

        observer.observe(document.querySelector("body"), config);
    }

    waitForMeets();

    function onArduinoPress(){
        if (document.querySelector("[role='button'][data-is-muted]").dataset.isMuted == "false") {
            document.querySelector("[role='button'][data-is-muted]").click()

        } else if (document.querySelector("[role='button'][data-is-muted]").dataset.isMuted == "true") {
            document.querySelector("[role='button'][data-is-muted]").click()
        }

    }

    async function readLoop(reader) {
        while (true && allowLoop) {
            const { value, done } = await reader.read();
            if (value) {
         
                let event = new CustomEvent("arduinoPress");
                document.dispatchEvent(event);
            }
            if (done) {
                reader.releaseLock();
                break;
            }
        }

    }

    async function connectSerial() {
        allowLoop = true;
        port = await window.navigator.serial.requestPort();

        await port.open({
            baudRate: 9600
        })

        let decoder = new TextDecoderStream();
        let inputDone = port.readable.pipeTo(decoder.writable);
        let inputStream = decoder.readable;
        reader = inputStream.getReader();


        const encoder = new TextEncoderStream();
        let outputDone = encoder.readable.pipeTo(port.writable);
        let outputStream = encoder.writable;
        const writer = await outputStream.getWriter();
        writer.write(target.dataset.isMuted == "true" ? "1" : "0");


        readLoop(reader);
        observeMutations(writer);
        streamWriter = writer;

        document.querySelector("#arduinoDisconnect").style.display="block";
        document.querySelector("#arduinoConnect").style.display="none";
        document.addEventListener("arduinoPress", onArduinoPress);

    }

    async function disconnectSerial() {
        try {
            allowLoop = false;
        await reader.cancel();
        await reader.releaseLock();
        await streamWriter.close()
        await streamWriter.releaseLock()
        streamWriter = null;
        await port.close();

        document.removeEventListener("arduinoPress", onArduinoPress);
        if(mutationObserver != null) mutationObserver.disconnect();

        document.querySelector("#arduinoDisconnect").style.display="none";
        document.querySelector("#arduinoConnect").style.display="block";
        }catch(e){

        }

    }

    function observeMutations(writer){
        let selector = "[role='button'][data-is-muted]";
        var target = document.querySelector(selector);


        var observer = new MutationObserver(function (mutations) {
            mutations.forEach(function (mutation) {
                if (mutation.attributeName == "data-is-muted" && streamWriter != null) {
                    streamWriter.write(target.dataset.isMuted == "true" ? "1" : "0");
                }
            });
        }
                                           );

        var config = {
            attributes: true,
            attributeFilter: ['data-is-muted'],
        };

        observer.observe(target, config);

        mutationObserver = observer;
    }


    document.addEventListener("targetAquired", function () {
        let genericStyle = `
    position: absolute;
    z-index: 123456;
    top: 0px;
    left: 0px;
    border: 0px;
    background-color: white;
    height: 48px;
    font-family: 'Google Sans',Roboto,Arial,sans-serif;
    font-size: 15px;
    font-weight: 400;
    letter-spacing: 0;
    line-height: 1.5rem;
    color: #202124;
    padding: 0px 10px;
    border-radius: 0px 0px 8px 00px;
`
        let button = document.createElement("button");
        button.id = "arduinoConnect";
        button.innerText = "Connect";
        button.style = genericStyle + "display:block;"
        button.onclick = connectSerial;
        document.body.insertAdjacentElement("afterbegin", button);


        button = document.createElement("button");
        button.id = "arduinoDisconnect";
        button.innerText = "Disconnect";
        button.style = genericStyle + "display:none;"
        button.onclick = disconnectSerial;
        document.body.insertAdjacentElement("afterbegin", button);
    })



})();
