function displayTime() {
  let date = new Date();
  let hh = date.getHours();
  let mm = date.getMinutes();
  let ss = date.getSeconds();
  let day = date.getDate();
  let month = date.getMonth() + 1; // Thang bat dau tu 0
  let year = date.getFullYear();

  // Các thu trong tuan
  const weekdays = [
    "Chủ Nhật",
    "Thứ Hai",
    "Thứ Ba",
    "Thứ Tư",
    "Thứ Năm",
    "Thứ Sáu",
    "Thứ Bảy",
  ];
  let weekday = weekdays[date.getDay()]; // Lay thu trong tuan

  let timeString = `${hh.toString().padStart(2, "0")}:${mm
    .toString()
    .padStart(2, "0")}:${ss.toString().padStart(2, "0")}`;
  let dateString = `${weekday} - ${day.toString().padStart(2, "0")}/${month
    .toString()
    .padStart(2, "0")}/${year}`;

  document.getElementById("timeLabel").textContent = timeString;
  document.getElementById("dateLabel").textContent = dateString;
}

setInterval(displayTime, 1000);
displayTime();

function showPage(pageId) {
  // An tat ca noi dung
  const contents = document.querySelectorAll(".content");
  contents.forEach((content) => content.classList.remove("active"));

  // Hien noi dung tuong ung
  document.getElementById(pageId).classList.add("active");
}

//menu
const menu = document.querySelector(".navbar__links");
const menuButton = document.querySelector(".navbar__icons");
const overlay = document.querySelector("#overlay");

menuButton.addEventListener("click", () => {
  menu.classList.toggle("navbar__open");
  menuButton.classList.toggle("open");
  overlay.classList.toggle("show");
});

// Weather
var main = document.querySelector("#name");
var temp = document.querySelector(".temp");
var desc = document.querySelector(".desc");

// Lấy api
const APP_ID = "29123485750d78a89be282224e74c41f";
function getWeather() {
  fetch(
    `https://api.openweathermap.org/data/2.5/weather?lat=10.76&lon=106.66&appid=${APP_ID}&units=metric`
  )
    .then((response) => response.json())
    .then((data) => {
      var tempValue = data["main"]["temp"];
      var nameValue = data["name"];
      var descValue = data["weather"][0]["description"];

      main.innerHTML = nameValue;
      desc.innerHTML = "Mô tả - " + descValue;
      temp.innerHTML = "Nhiệt độ - " + tempValue + " °C";
    })
    .catch((err) => alert("Không thể lấy thông tin thời tiết!"));
}

// Get weather Ho Chi Minh City when open website
getWeather();

function led(show) {
  let pic;
  if (show == 0) {
    pic = "img/bulboff.gif";
    firebase.database().ref("led").set({ ledState: false });
  } else {
    pic = "img/bulbon.gif";
    firebase.database().ref("led").set({ ledState: true });
  }

  // Update the image source immediately
  document.getElementById("bulb").src = pic;

  // Optionally, update based on the database state
  // firebase
  //   .database()
  //   .ref("led/ledState")
  //   .once("value")
  //   .then((snapshot) => {
  //     const ledState = snapshot.val();
  //     if (ledState) {
  //       document.getElementById("bulb").src = "img/bulbon.gif";
  //     } else {
  //       document.getElementById("bulb").src = "img/bulboff.gif";
  //     }
  //   })
  //   .catch((error) => {
  //     console.error("Error fetching LED state:", error);
  //   });
}
