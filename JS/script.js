const texts = ["Web development", "Electronics"];
const typingSpeed = 45;
const pause = 1100;

const projects = [
    {
        name: "FPV Car Web Controller",
        category: "electronics",
        tag: "Electronics + Web",
        progress: 96,
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
        category: "web",
        tag: "Web",
        progress: 88,
        description: "A GitHub Pages portfolio with a dynamic page system, animated hero, project showcase, and contact flow.",
        features: [
            "Single-page dynamic navigation",
            "Responsive mobile menu",
            "Project showcase and progress control",
            "Professional dark portfolio theme"
        ],
        link: "",
        linkText: ""
    },
    {
        name: "Electronics Lab",
        category: "electronics",
        tag: "Electronics",
        progress: 42,
        description: "A growing space for electronics builds, hardware tests, embedded control ideas, and future experiments.",
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

let currentTextIndex = 0;
let isDeleting = false;
let currentText = "";
let activeProjectIndex = 0;

const displayElement = document.getElementById("type_effect");
const pageViews = document.querySelectorAll(".page_view");
const navButtons = document.querySelectorAll("[data-page]");
const menuToggle = document.querySelector(".menu_toggle");
const navMenu = document.querySelector("#nav_menu");
const projectCards = document.querySelectorAll(".cards");
const previousProject = document.querySelector(".arrow_left");
const nextProject = document.querySelector(".arrow_right");
const projectTitle = document.querySelector(".project_name");
const projectDescription = document.querySelector(".description");
const projectFeatures = document.querySelector(".project_feature ol");
const projectRing = document.querySelector(".project_details .progress_ring");
const projectProgressValue = document.querySelector(".project_details .progress_value");
const projectLink = document.querySelector(".project_open_link");

function type() {
    if (!displayElement) {
        return;
    }

    const targetText = texts[currentTextIndex];
    currentText = isDeleting
        ? targetText.substring(0, currentText.length - 1)
        : targetText.substring(0, currentText.length + 1);

    displayElement.textContent = currentText;

    if (!isDeleting && currentText === targetText) {
        isDeleting = true;
        setTimeout(type, pause);
        return;
    }

    if (isDeleting && currentText === "") {
        isDeleting = false;
        currentTextIndex = (currentTextIndex + 1) % texts.length;
        setTimeout(type, pause);
        return;
    }

    setTimeout(type, typingSpeed);
}

function setProgress(ring, valueElement, progress) {
    ring.style.setProperty("--progress", progress);
    valueElement.textContent = `${progress}%`;
}

function updateProject(index) {
    activeProjectIndex = (index + projects.length) % projects.length;
    const project = projects[activeProjectIndex];

    projectCards.forEach((card, cardIndex) => {
        card.classList.toggle("active_card", cardIndex === activeProjectIndex);
    });

    projectTitle.textContent = project.name;
    projectDescription.textContent = project.description;
    projectFeatures.replaceChildren(...project.features.map((feature) => {
        const item = document.createElement("li");
        item.textContent = feature;
        return item;
    }));

    setProgress(projectRing, projectProgressValue, project.progress);
    projectLink.href = project.link;
    projectLink.textContent = project.linkText;
    projectLink.style.display = project.link ? "inline-flex" : "none";
}

function closeMenu() {
    navMenu.classList.remove("open");
    menuToggle.setAttribute("aria-expanded", "false");
}

function showPage(pageName) {
    const viewName = pageName === "home" ? "home" : pageName;

    pageViews.forEach((view) => {
        view.classList.toggle("active_page", view.dataset.view === viewName);
    });

    document.querySelectorAll(".nav_btn").forEach((button) => {
        button.classList.toggle("active_nav", button.dataset.page === pageName);
    });

    closeMenu();
    window.scrollTo({ top: 0, behavior: "smooth" });
}

function createProjectTile(project) {
    const tile = document.createElement("article");
    tile.className = "project_tile";

    const top = document.createElement("div");
    top.className = "tile_top";

    const tag = document.createElement("div");
    tag.className = "card_tag";
    tag.textContent = project.tag;

    const title = document.createElement("h2");
    title.textContent = project.name;

    const description = document.createElement("p");
    description.className = "description";
    description.textContent = project.description;

    const ring = document.createElement("div");
    ring.className = "progress_ring tile_progress";
    ring.style.setProperty("--progress", project.progress);

    const value = document.createElement("span");
    value.className = "progress_value";
    value.textContent = `${project.progress}%`;
    ring.append(value);

    const features = document.createElement("ol");
    features.className = "tile_features";
    project.features.forEach((feature) => {
        const item = document.createElement("li");
        item.textContent = feature;
        features.append(item);
    });

    top.append(tag, title, description, ring, features);
    tile.append(top);

    if (project.link) {
        const link = document.createElement("a");
        link.className = "project_open_link";
        link.href = project.link;
        link.textContent = project.linkText;
        tile.append(link);
    }

    return tile;
}

function renderProjectPages() {
    const webGrid = document.querySelector("#web_project_grid");
    const electronicsGrid = document.querySelector("#electronics_project_grid");

    projects
        .filter((project) => project.category === "web")
        .forEach((project) => webGrid.append(createProjectTile(project)));

    projects
        .filter((project) => project.category === "electronics")
        .forEach((project) => electronicsGrid.append(createProjectTile(project)));
}

menuToggle.addEventListener("click", () => {
    const isOpen = navMenu.classList.toggle("open");
    menuToggle.setAttribute("aria-expanded", String(isOpen));
});

navButtons.forEach((button) => {
    button.addEventListener("click", () => {
        const page = button.dataset.page;

        if (page === "home") {
            showPage("home");
            return;
        }

        showPage(page);
    });
});

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
            button.textContent = "Copied";
            setTimeout(() => {
                button.textContent = "Copy";
            }, 1200);
        } catch (error) {
            button.textContent = "Select";
        }
    });
});

document.addEventListener("DOMContentLoaded", () => {
    type();
    updateProject(0);
    renderProjectPages();
});
