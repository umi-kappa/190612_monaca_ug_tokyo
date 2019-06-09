/**
 * サービスのUUID
 * @type {string}
 */
const SERVICE_UUID = '8a61d7f7-888e-4e72-93be-0df87152fc6d';

/**
 * キャラクタリスティックのUUID
 * @type {string}
 */
const CHARACTERISTIC_UUID = 'fae2e24f-aea2-48cb-b449-55ec20518e93';

/**
 * 扱うセンサー値の上限
 * @type {number}
 */
const SENSOR_MAX_VALUE = 130;

/**
 * 扱うセンサー値の下限
 * @type {number}
 */
const SENSOR_MIN_VALUE = 50;

/**
 * BLEに接続するボタン
 */
let connectButton;

/**
 * ビール全体のDOM
 */
let beerMain;

/**
 * ビールグラスのDOM
 */
let beerMask;

/**
 * ローディングボタン
 */
let loading;

/**
 * 心拍数表示用Div
 */
let heartRateText;

/**
 * BLE経由で読み込んだセンサー値
 */
let sensorValue;

/**
 * ビールグラスの表示率
 */
let beerPer;

/**
 * 初期化処理
 */
const init = () => {
  // beerの拡大率を算出
  let designWidth = 458;
  let designHeight = 755;
  let margin = 40;
  let windowW = window.innerWidth - margin;
  let windowH = window.innerHeight - margin;

  if (windowW < windowH && windowW / designWidth * designHeight < windowH) {
    this.wrapperScale = windowW / designWidth;
  } else {
    this.wrapperScale = windowH / designHeight;
  }

  let beerMainInner = document.querySelector('#beer-main-inner');
  beerMainInner.style.transform = 'scale(' + this.wrapperScale + ')';

  sensorValue = 0;
  beerPer = 0;

  beerMain = document.querySelector('#beer-main');
  beerMask = document.querySelector('#beer-mask');

  loading = document.querySelector('#loading');
  heartRateText = document.querySelector('#heart-rate-text');

  connectButton = document.querySelector('#ble-connect-button');
  connectButton.addEventListener('click', () => {
    connect();
  });
};

/**
 * Web Bluetooth APIでBLEデバイスに接続します。
 */
const connect = () => {
  // loading表示
  loading.className = 'show';

  navigator.bluetooth.requestDevice({
    filters: [
      {services: [SERVICE_UUID]},
      {namePrefix: 'm5-stack'}
    ],
    optionalServices: [
      // 使用したいServiceを登録しておく
      SERVICE_UUID
    ]
  })
    .then(device => {
      console.log('デバイスを選択しました。接続します。');
      console.log('デバイス名 : ' + device.name);
      console.log('ID : ' + device.id);

      // 選択したデバイスに接続
      return device.gatt.connect();
    })
    .then(server => {
      console.log('デバイスへの接続に成功しました。サービスを取得します。');

      // UUIDに合致するサービス(機能)を取得
      return server.getPrimaryService(SERVICE_UUID);
    })
    .then(service => {
      console.log('サービスの取得に成功しました。キャラクタリスティックを取得します。');

      // UUIDに合致するキャラクタリスティック(サービスが扱うデータ)を取得
      return service.getCharacteristic(CHARACTERISTIC_UUID);
    })
    .then(characteristic => {
      console.log('BLE接続が完了しました。');

      // アルコール値を表示
      showMainView();

      // ビール更新
      loop();

      return setNotifications(characteristic);
    })
    .catch(error => {
      console.log('Error : ' + error);

      // loading非表示
      loading.className = 'hide';
    });
};

const setNotifications = (characteristic) => {
  // Notifications開始
  return characteristic.startNotifications()
    .then(() => {
      // Add Event
      characteristic.addEventListener('characteristicvaluechanged', (event) => {
        const value = event.target.value;

        // 心拍数を取得
        sensorValue = value.getUint8(0);
        heartRateText.innerText = sensorValue;
      });
    })
};

const loop = () => {
  beerPer += ((sensorValue - SENSOR_MIN_VALUE) / (SENSOR_MAX_VALUE - SENSOR_MIN_VALUE) * 100 - beerPer) * 0.1;
  beerMask.style.height = beerPer + '%';

  window.requestAnimationFrame(loop);
};

/**
 * 心拍値を表示します。
 */
const showMainView = () => {
  // 接続ボタン
  connectButton.className = 'hide';
  // loading非表示
  loading.className = 'hide';
  // ビール表示
  beerMain.className = 'show';
  // 心拍数表示
  heartRateText.className = 'show';
};

init();
