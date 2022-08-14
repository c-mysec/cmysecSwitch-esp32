/**
 * ===============================
 * Notify-JS
 * Simple vanilla JS notifications.
 * ===============================
 * Feel free to use how you like :)
 * https://github.com/baswilson/notfiy-js
 */

var notifications = [];

const styles = `
.notifications-container {
    position: fixed;
    display: flex;
    flex-direction: column;
    top: 20px;
    right: 20px;
    max-width: 30%;
    align-items: flex-end;
    z-index: 10000;
}

.notification-container {
    width: fit-content;
    padding: 20px;
    text-align: left;
    height: fit-content;
    cursor: pointer;
    margin-bottom: 10px;
    border-radius: 3px;
  }
  
  .notification-content p {
    margin: 0px;
    font-size: 14px;
    line-height: 20px;
    height: fit-content;
    color: white;
  }
`;

class NotifyJS {

  /**
   * @param {Object} settings 
   * @param {Object} style 
   */
  constructor(settings = { duration: 3000, message: 'NotifyJS Notification', timer: false }, style = { color: 'crimson', textColor: 'white' }) {

    // Settings
    this.duration = typeof settings.duration == 'undefined' ? 3000 : settings.duration;
    this.message = typeof settings.message == 'undefined' ? 'NotifyJS Notification' : settings.message;
    this.timer = typeof settings.timer == 'undefined' ? false : settings.timer;

    // Style
    this.color = typeof style.color == 'undefined' ? 'crimson' : style.color;
    this.textColor = typeof style.textColor == 'undefined' ? 'white' : style.textColor;
    this.fontFamily = typeof style.fontFamily == 'undefined' ? 'Verdana' : style.fontFamily;
    this.customCSSBox = typeof style.customCSSBox == 'undefined' ? '' : style.customCSSBox;

    this.interval;
    this.id = notifications.length;
    this.create();
  }

  create() {
    notifications.push(this);

    if (document.getElementById('notifications') == null) {

      var css = document.createElement('style');
      css.type = 'text/css';

      if (css.styleSheet) css.styleSheet.cssText = styles;
      else css.appendChild(document.createTextNode(styles));

      document.getElementsByTagName("head")[0].appendChild(css);

      document.body.innerHTML += `
        <!-- Notification -->
        <div class="notifications-container" id="notifications"></div>
        <!-- Notification end -->
        `;
    }
    document.getElementById('notifications').innerHTML += `
    <div onclick="deleteNotification(${this.id})" id="notification-${this.id}" class="notification-container" style="background-color: ${this.color}; ${this.customCSSBox}">
      <div class="notification-content">
          <p id="notification-text" style="color: ${this.textColor}; font-family: ${this.fontFamily}">${this.message}</p>
          ${this.timer ? `<p id="removing-in-${this.id}" style="color: ${this.textColor}; font-family: ${this.fontFamily}" class="hint" >${this.duration / 1000}s</p>` : ''}
      </div>
    </div>
    `;

    this.interval = setInterval(() => {
      this.duration -= 1000;
      this.timer ? document.getElementById(`removing-in-${this.id}`).innerHTML = `${this.duration / 1000}s` : '';
    }, 1000);

    this.timeout = setTimeout(() => {
      this.delete();
    }, this.duration);
  }

  delete() {
    notifications.splice(this, 1);
    clearInterval(this.interval);
    clearTimeout(this.timeout);
    document.getElementById(`notification-${this.id}`).remove();
  }
}

function deleteNotification(id) {
  for (let i = 0; i < notifications.length; i++) {
    if (notifications[i].id == id) {
      return notifications[i].delete();
    }
  }
}