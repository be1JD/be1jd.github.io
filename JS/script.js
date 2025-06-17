// For Typing Effect
const texts = ["Web development", "Electronics"];
let currentTextIndex = 0;
let isDeleting = false;
let currentText = "";
const typingSpeed = 30; // Speed in milliseconds
const pause = 1000; // Pause between typing and deleting

// Typing Effect Function
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

// calling typing effect function
document.addEventListener("DOMContentLoaded", type);

// Adding About me page

// NOT mastered this technique yet
// let aboutme = document.querySelectorAll(".nav_btn");
// console.log(aboutme)
// aboutme.forEach(element => {
// element.addEventListener("click", (e)=>{
// console.log(e.currentTarget)
// console.log("pressed")
// })
// });

// Lengthy code but reasonalble right now 
let myproject = document.querySelector("#myproject");
let electronics = document.querySelector("#electronics");
let aboutme = document.querySelector("#aboutme");
let contact = document.querySelector("#contact");

// My Project button
myproject.addEventListener("click", () => {
    console.log("Pressed myproject");
})

// Electrnoics button 
electronics.addEventListener("click", () => {
    console.log("Pressed electronics");
})

// About me button
aboutme.addEventListener("click", () => {
    console.log("Pressed aboutme");
    let main_tag = document.querySelector("main");
    main_tag.style.display = "none"
})

// Contact button
contact.addEventListener("click", () => {
    console.log("Pressed contact");
    let main_tag = document.querySelector("main");
    main_tag.style.display = "none"
})



