// Constantes para mensajes y estados
const socketMessage = {
    STREAM: 'STREAM',
    CONFIG: 'CONFIG',
    COMMAND: 'COMMAND',
    END_CONNECTION: 'END_CONNECTION'
};

const socketMessageConfig = {
    FRAMESIZE: 'FRAMESIZE',
    LED: 'LED',
    HMIRROR: 'HMIRROR',
    VFLIP: 'VFLIP'
};

const socketMessageStream = {
    PAUSE: 'PAUSE',
    START: 'START'
};

const btnGrant = document.querySelector('#btn-grant');
const btnDeny = document.querySelector('#btn-deny');

const streamImage = document.querySelector('#stream-image');

const btnOpenModal = document.querySelector('#btn-open-modal');
const btnCloseModal = document.querySelector('#btn-close-modal');
const modalConfig = document.querySelector('#modal-config');

const formConfig = document.querySelector('#form-config');
const framesizeSelect = document.querySelector('#framesize-select');
const ledCheckbox = document.querySelector('#led-checkbox');
const hmirrorCheckbox = document.querySelector('#hmirror-checkbox');
const vflipCheckbox = document.querySelector('#vflip-checkbox');
const btnSaveConfig = document.querySelector('#btn-save-config');

let isWaitingStream = true;
let isWaitingConfig = true;
const config = { 
    framesize: '8',
    led: '0',
    hmirror: '1',
    vflip: '0',
};

const cameraSocket = new WebSocket(wsURL);

setIsWaitingConfig(true);
handleStreamStatus(socketMessageStream.PAUSE);

cameraSocket.addEventListener("message", handleCameraSocketMessage);

formConfig.addEventListener('submit', event => event.preventDefault());

btnGrant.addEventListener('click', handleGrant);
btnDeny.addEventListener('click', handleDeny);

btnSaveConfig.addEventListener('click', handleSaveConfig);
btnOpenModal.addEventListener('click', toggleModalConfig);
btnCloseModal.addEventListener('click', toggleModalConfig);

async function handleGrant() {
    await fetch('{{grantUrl}}');
    closeCameraSocket();
    window.location.reload();
}

async function handleDeny() {
    await fetch('{{denyUrl}}');
    closeCameraSocket();
    window.location.reload();
}

function handleSaveConfig() {
    if (!isWaitingConfig) {
        handleConfigSubmit();
    } else {
        toggleModalConfig();
    }
}

function handleCameraSocketMessage(event) {
    if (typeof event.data === 'string') {
        handleStringMessage(event.data);
    } else {
        handleStreamMessage(event.data);
    }
}

function closeCameraSocket() {
    sendMessage({ messageType: socketMessage.END_CONNECTION });
    cameraSocket.close();
}

function sendMessage({ messageType, messageValue = '' }) {
    if (!messageType || cameraSocket.readyState !== WebSocket.OPEN) {
        return;
    }
    const message = `[${messageType}]${messageValue}`;
    cameraSocket.send(message);
}

function handleStringMessage(message) {
    const messagePattern = /^\[(\w+)\](.*)$/;
    const [, messageType, messageValue] = message.match(messagePattern) || [];

    if (messageType === socketMessage.STREAM) {
        handleStreamStatus(messageValue);
    } else if (messageType === socketMessage.CONFIG) {
        handleConfigMessage(messageValue);
    } else {
        console.log('Unknown message:', { message, messageType, messageValue });
    }
}

function handleConfigMessage(message) {
    const configValues = message.split(',');

    configValues.forEach(value => {
        const [configType, configValue] = value.split(':');
        if (configType === socketMessageConfig.FRAMESIZE) {
            config.framesize = configValue;
            framesizeSelect.value = configValue;
        } else if (configType === socketMessageConfig.LED) {
            config.led = configValue;
            ledCheckbox.checked = configValue === '1';
        } else if (configType === socketMessageConfig.HMIRROR) {
            config.hmirror = configValue;
            hmirrorCheckbox.checked = configValue === '1';
        } else if (configType === socketMessageConfig.VFLIP) {
            config.vflip = configValue;
            vflipCheckbox.checked = configValue === '1';
        } else {
            console.log('Unknown config:', { configType, configValue });
        }
    });

    setIsWaitingConfig(false);
}

async function handleStreamMessage(message) {
    if (isWaitingStream) {
        handleStreamStatus(socketMessageStream.START);
    }

    const blob = new Blob([message], { type: 'image/jpeg' });

    const bitmap = await createImageBitmap(blob);

    const canvas = document.createElement('canvas');
    canvas.width = bitmap.width;
    canvas.height = bitmap.height;
    const context = canvas.getContext('2d');
    context.drawImage(bitmap, 0, 0);

    streamImage.src = canvas.toDataURL();

    if (streamImage.src) {
        URL.revokeObjectURL(streamImage.src);
    }
}

function handleStreamStatus(status) {
    if (status === socketMessageStream.PAUSE) {
        streamImage.classList.add('charging');
        isWaitingStream = true;
    } else if (status === socketMessageStream.START) {
        streamImage.classList.remove('charging');
        isWaitingStream = false;
    }
}

function handleConfigSubmit() {
    const formValues = new FormData(formConfig);
    const sendData = [];

    const framesize = formValues.get('framesize');
    if (framesize !== config.framesize) {
        sendData.push(`${socketMessageConfig.FRAMESIZE}:${framesize}`);
    }

    const led = formValues.get('led') === 'on' ? '1' : '0';
    if (led !== config.led) {
        sendData.push(`${socketMessageConfig.LED}:${led}`);
    }

    const hmirror = formValues.get('hmirror') === 'on' ? '1' : '0';
    if (hmirror !== config.hmirror) {
        sendData.push(`${socketMessageConfig.HMIRROR}:${hmirror}`);
    }

    const vflip = formValues.get('vflip') === 'on' ? '1' : '0';
    if (vflip !== config.vflip) {
        sendData.push(`${socketMessageConfig.VFLIP}:${vflip}`);
    }

    if (sendData.length > 0) {
        setIsWaitingConfig(true);
        sendMessage({ messageType: socketMessage.COMMAND, messageValue: sendData.join(',') });
    }
}

function setIsWaitingConfig(value) {
    if (value) {
        modalConfig.classList.add('charging');
        formConfig.classList.add('disabled');
        btnSaveConfig.textContent = 'Cerrar';
    } else {
        modalConfig.classList.remove('charging');
        formConfig.classList.remove('disabled');
        btnSaveConfig.textContent = 'Guardar';
    }
    isWaitingConfig = value;
}

function toggleModalConfig() {
    modalConfig.classList.toggle('hidden');
}
