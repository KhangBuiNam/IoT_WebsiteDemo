function displayTime() {
  let date = new Date();
  let hh = date.getHours();
  let mm = date.getMinutes();
  let ss = date.getSeconds();
  let day = date.getDate();
  let month = date.getMonth() + 1; // Month bat dau tu 0
  let year = date.getFullYear();

  // Cac thu trong tuan
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
  let dateString = `${day.toString().padStart(2, "0")}/${month
    .toString()
    .padStart(2, "0")}/${year}`;

  document.getElementById(
    "timeLabel"
  ).textContent = `${timeString} - ${weekday} - ${dateString} `;
}

setInterval(displayTime, 1000);
displayTime();

function showPage(pageId) {
  // An tat ca noi dung
  const contents = document.querySelectorAll(".content");
  contents.forEach((content) => content.classList.remove("active"));

  // Hiện nội dung tương ứng
  document.getElementById(pageId).classList.add("active");
}
