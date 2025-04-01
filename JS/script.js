const texts = ["Web development", "Electronics"];
let currentTextIndex = 0;
let isDeleting = false;
let currentText = "";
const typingSpeed = 30; // Speed in milliseconds
const pause = 1000; // Pause between typing and deleting

function type() {
    const targetText = texts[currentTextIndex];
    const displayElement = document.getElementById("type_effect");

    if (isDeleting) {
        currentText = targetText.substring(0, currentText.length - 1);
    } else {
        currentText = targetText.substring(0, currentText.length + 1);
    }

    displayElement.innerHTML = currentText;

    if (!isDeleting && currentText === targetText) {
        isDeleting = true;
        setTimeout(type, pause); // Pause before starting to delete
    } else if (isDeleting && currentText === "") {
        isDeleting = false;
        currentTextIndex = (currentTextIndex + 1) % texts.length;
        setTimeout(type, pause); // Pause before starting to type the next text
    } else {
        setTimeout(type, typingSpeed);
    }
}

document.addEventListener("DOMContentLoaded", type);