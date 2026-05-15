// For Typing Effect
const texts = ["Web development", "Electronics"];
let currentTextIndex = 0;
let isDeleting = false;
let currentText = "";
const typingSpeed = 30; // Speed in milliseconds
const pause = 1000; // Pause between typing and deleting
const projects = [
    {
        name: "FPV Car Web Controller",
        description: "A static, mobile-first controller page for an FPV RC car. It is GitHub Pages friendly and connects to the car over WebSocket when hosted on the device network.",
        features: [
            "Touch steering and speed control",
            "Forward, reverse, brake, and center actions",
            "Status and debug panels",
            "Static HTML deployment support"
        ],
        link: "FPV_Car/RC_Control_4.3.html",
        linkText: "Launch FPV Controller"
    },
    {
        name: "Portfolio Website",
        description: "A lightweight static website for showing web development and electronics experiments.",
        features: [
            "Static GitHub Pages hosting",
            "Animated hero typing effect",
            "Project showcase section",
            "Contact and about sections"
        ],
        link: "#projects",
        linkText: "View Project Section"
    },
    {
        name: "Electronics Lab",
        description: "A growing space for electronics builds, hardware tests, and embedded control ideas.",
        features: [
            "Hardware project notes",
            "Controller experiments",
            "Web-connected electronics ideas",
            "Future project showcase slot"
        ],
        link: "FPV_Car/RC_Control_4.3.html",
        linkText: "See FPV Example"
    }
];
let activeProjectIndex = 0;

// Get all the Buttons on Nav bar
let home = document.querySelector("#home");
let myproject = document.querySelector("#myproject");
let electronics = document.querySelector("#electronics");
let aboutme = document.querySelector("#aboutme");
let contact = document.querySelector("#contact");

//Element Containing Main pages
let main_tag = document.querySelector("main");
let contact_page = document.querySelector(".contact_page_enable")
let aboutme_page = document.querySelector(".aboutme_page_enable")
let projectCards = document.querySelectorAll(".cards");
let previousProject = document.querySelector(".arrow_left");
let nextProject = document.querySelector(".arrow_right");
let projectTitle = document.querySelector(".prject_name h3");
let projectDescription = document.querySelector(".desctiption");
let projectFeatures = document.querySelector(".project_feature ul");
let projectMeter = document.querySelector(".progress_meter");
let projectLink = document.querySelector(".project_open_link");

// Typing Effect Function
function type() {
    const targetText = texts[currentTextIndex];
    const displayElement = document.getElementById("type_effect");

    if (isDeleting) {
        currentText = targetText.substring(0, currentText.length - 1);
    } else {
        currentText = targetText.substring(0, currentText.length + 1);
    }

    displayElement.textContent = currentText;

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

function showMainPage() {
    main_tag.style.display = "block";
    contact_page.style.display = "none";
    aboutme_page.style.display = "none";
}

function updateProject(index, scrollCards = true) {
    activeProjectIndex = (index + projects.length) % projects.length;
    const project = projects[activeProjectIndex];

    projectCards.forEach((card, cardIndex) => {
        card.classList.toggle("active_card", cardIndex === activeProjectIndex);
    });
    if (scrollCards) {
        projectCards[activeProjectIndex].scrollIntoView({ behavior: "smooth", block: "nearest", inline: "center" });
    }

    projectTitle.textContent = project.name;
    projectDescription.textContent = project.description;
    projectFeatures.replaceChildren(...project.features.map((feature) => {
        const item = document.createElement("li");
        item.textContent = feature;
        return item;
    }));
    projectMeter.textContent = `${activeProjectIndex + 1} / ${projects.length}`;
    projectLink.href = project.link;
    projectLink.textContent = project.linkText;
}

function scrollToProjects() {
    showMainPage();
    document.querySelector("#projects").scrollIntoView({ behavior: "smooth", block: "start" });
}

projectCards.forEach((card) => {
    card.addEventListener("click", (event) => {
        if (event.target.closest("a")) {
            return;
        }

        updateProject(Number(card.dataset.project));
    });
});

previousProject.addEventListener("click", () => {
    updateProject(activeProjectIndex - 1);
});

nextProject.addEventListener("click", () => {
    updateProject(activeProjectIndex + 1);
});

document.querySelectorAll(".copy_btn").forEach((button) => {
    button.addEventListener("click", async () => {
        const text = button.parentElement.querySelector(".copy_text").textContent.trim();

        try {
            await navigator.clipboard.writeText(text);
            button.textContent = "copied";
            setTimeout(() => {
                button.textContent = "copy";
            }, 1200);
        } catch (error) {
            button.textContent = "select";
        }
    });
});

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
// Contact button
home.addEventListener("click", () => {
    showMainPage();
    window.scrollTo({ top: 0, behavior: "smooth" });
})

// My Project button
myproject.addEventListener("click", () => {
    scrollToProjects();
})

// Electrnoics button 
electronics.addEventListener("click", () => {
    scrollToProjects();
    updateProject(0);
})

// About me button
aboutme.addEventListener("click", () => {
    main_tag.style.display = "none"
    contact_page.style.display = "none"
    aboutme_page.style.display = "block"
})

// Contact button
contact.addEventListener("click", () => {
    main_tag.style.display = "none"
    contact_page.style.display = "block"
    aboutme_page.style.display = "none"
})

updateProject(0, false);



