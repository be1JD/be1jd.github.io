const fallbackImage = "Assets/Mypic.jpg";

const heroImages = [
    "Hero Images/hero-1.jpg",
    "Hero Images/hero-2.jpg",
    "Hero Images/hero-3.jpg",
    "Hero Images/1.jpg",
    "Hero Images/2.jpg",
    "Assets/Mypic.jpg"
];

const projects = [
    {
        id: "fpv-car",
        name: "FPV Car Web Controller",
        tag: "Electronics + Web",
        progress: 96,
        price: "Custom quote",
        folder: "FPV_Car",
        liveLink: "FPV_Car/RC_Control_4.3.html",
        description: "Browser-based driving HUD for an FPV RC car with steering, speed, brake, and settings controls.",
        overview: "This project is designed as a static browser interface, so it can run from GitHub Pages or from a device-hosted page without server-side processing.",
        features: ["Touch controls", "WebSocket ready", "Mobile-first HUD", "Static deployment"],
        steps: [
            "Open the controller page in the same browser tab.",
            "Connect to the car network or controller endpoint.",
            "Use the touch controls to steer, brake, and tune the drive behavior."
        ],
        images: [
            "Projects/Project_1/Images/cover.png",
            "Projects/Project_1/Images/1.png",
            "Projects/Project_1/Images/2.jpg",
            "Projects/Project1/Images/cover.jpg",     
             fallbackImage]
    },
    {
        id: "project-1",
        name: "Project 1",
        tag: "Project Build",
        progress: 70,
        price: "Available on request",
        folder: "Projects/Project_1",
        liveLink: "Projects/Project_1/index.html",
        description: "A project slot connected to Projects/Project_1 with a swipeable image gallery and guide-style detail page.",
        overview: "Add images inside Projects/Project_1/Images and update this project entry with your final name, description, price, and steps.",
        features: ["Swipe gallery", "Guide layout", "Buy request", "GitHub Pages ready"],
        steps: [
            "Place project images in Projects/Project_1/Images.",
            "Add the project HTML page at Projects/Project_1/index.html if this build has its own code.",
            "Update the manifest in JS/script.js with final project details."
        ],
        images: [
            "Projects/Project_1/Images/cover.jpg",
            "Projects/Project_1/Images/1.jpg",
            "Projects/Project_1/Images/2.jpg",
            "Projects/Project1/Images/cover.jpg",
            fallbackImage
        ]
    },
    {
        id: "project-2",
        name: "Project 2",
        tag: "Project Build",
        progress: 35,
        price: "Available on request",
        folder: "Projects/Project_2",
        liveLink: "Projects/Project_2/index.html",
        description: "A second project slot connected to Projects/Project_2 with the same clean gallery, guide, and purchase flow.",
        overview: "This entry is ready for the next project folder. Keep everything static and browser-side for GitHub Pages hosting.",
        features: ["Project manifest", "Responsive layout", "Detail page", "Contact CTA"],
        steps: [
            "Place images in Projects/Project_2/Images.",
            "Add the project page at Projects/Project_2/index.html when needed.",
            "Replace this placeholder content with your project details."
        ],
        images: [
            "Projects/Project_2/Images/cover.jpg",
            "Projects/Project_2/Images/1.jpg",
            "Projects/Project_2/Images/2.jpg",
            "Projects/Project2/Images/cover.jpg",
            fallbackImage
        ]
    }
];

let activeProjectIndex = 0;
let activeGalleryIndex = 0;
let heroIndex = 0;
let touchStartX = 0;

const pageViews = document.querySelectorAll(".page_view");
const navMenu = document.querySelector("#nav_menu");
const menuToggle = document.querySelector(".menu_toggle");
const heroImage = document.querySelector("#hero_image");
const featuredImage = document.querySelector("#featured_project_image");
const featuredProgress = document.querySelector("#featured_progress");
const featuredMeta = document.querySelector("#featured_meta");
const featuredTitle = document.querySelector("#featured_title");
const featuredDescription = document.querySelector("#featured_description");
const featuredFeatures = document.querySelector("#featured_features");
const featuredOpen = document.querySelector("#featured_open");
const featuredPager = document.querySelector("#featured_pager");
const projectList = document.querySelector("#project_list");
const gallery = document.querySelector("#home_project_gallery");

function closeMenu() {
    navMenu.classList.remove("open");
    menuToggle.setAttribute("aria-expanded", "false");
}

function showPage(pageName) {
    pageViews.forEach((view) => {
        view.classList.toggle("active_page", view.dataset.view === pageName);
    });

    document.querySelectorAll(".nav_btn").forEach((button) => {
        button.classList.toggle("active_nav", button.dataset.page === pageName);
    });

    closeMenu();
    window.scrollTo({ top: 0, behavior: "smooth" });
}

function imageExists(src) {
    return new Promise((resolve) => {
        const image = new Image();
        image.onload = () => resolve(src);
        image.onerror = () => resolve("");
        image.src = src;
    });
}

async function setImageFromCandidates(imageElement, candidates, altText) {
    const sources = [...new Set([...candidates, fallbackImage])];

    for (const source of sources) {
        const loaded = await imageExists(source);
        if (loaded) {
            imageElement.src = loaded;
            imageElement.alt = altText;
            return loaded;
        }
    }

    imageElement.src = fallbackImage;
    imageElement.alt = altText;
    return fallbackImage;
}

function setProgress(element, progress) {
    element.style.setProperty("--progress", progress);
    element.querySelector("span").textContent = `${progress}%`;
}

function openProject(project) {
    renderProjectDetail(project);
    showPage("project-detail");
}

function updateFeaturedProject(index) {
    activeProjectIndex = (index + projects.length) % projects.length;
    activeGalleryIndex = 0;

    const project = projects[activeProjectIndex];
    featuredMeta.textContent = project.tag;
    featuredTitle.textContent = project.name;
    featuredDescription.textContent = project.description;
    setProgress(featuredProgress, project.progress);

    featuredFeatures.replaceChildren(...project.features.map((feature) => {
        const item = document.createElement("span");
        item.textContent = feature;
        return item;
    }));

    featuredOpen.onclick = () => openProject(project);
    renderFeaturedPager();
    updateFeaturedImage();
}

function updateFeaturedImage() {
    const project = projects[activeProjectIndex];
    const orderedImages = [
        project.images[activeGalleryIndex],
        ...project.images.filter((_, index) => index !== activeGalleryIndex)
    ];

    setImageFromCandidates(featuredImage, orderedImages, `${project.name} preview`);
}

function moveGallery(direction) {
    const imageCount = projects[activeProjectIndex].images.length;
    activeGalleryIndex = (activeGalleryIndex + direction + imageCount) % imageCount;
    updateFeaturedImage();
}

function moveFeaturedProject(direction) {
    updateFeaturedProject(
        activeProjectIndex + direction
    );
}

function renderFeaturedPager() {
    featuredPager.replaceChildren(...projects.map((project, index) => {
        const button = document.createElement("button");
        button.type = "button";
        button.className = index === activeProjectIndex ? "active" : "";
        button.setAttribute("aria-label", `Show ${project.name}`);
        button.addEventListener("click", () => updateFeaturedProject(index));
        return button;
    }));
}

async function createProjectRow(project, index) {
    const row = document.createElement("button");
    row.className = "project_row";
    row.type = "button";
    row.addEventListener("click", () => openProject(project));

    const imageWrap = document.createElement("div");
    imageWrap.className = "project_row_image";

    const image = document.createElement("img");
    image.alt = `${project.name} thumbnail`;
    imageWrap.append(image);
    setImageFromCandidates(image, project.images, `${project.name} thumbnail`);

    const content = document.createElement("div");
    content.className = "project_row_content";

    const meta = document.createElement("p");
    meta.className = "project_meta";
    meta.textContent = `${project.tag} / ${project.progress}% complete`;

    const title = document.createElement("h2");
    title.textContent = project.name;

    const description = document.createElement("p");
    description.textContent = project.description;

    content.append(meta, title, description);
    row.append(imageWrap, content);

    row.style.animationDelay = `${index * 60}ms`;
    return row;
}

async function renderProjectList() {
    const rows = await Promise.all(projects.map(createProjectRow));
    projectList.replaceChildren(...rows);
}

function renderProjectDetail(project) {
    document.querySelector("#detail_meta").textContent = `${project.tag} / ${project.folder}`;
    document.querySelector("#detail_title").textContent = project.name;
    document.querySelector("#detail_description").textContent = project.description;
    document.querySelector("#detail_overview").textContent = project.overview;
    document.querySelector("#detail_price").textContent = project.price;
    setProgress(document.querySelector("#detail_progress"), project.progress);
    setImageFromCandidates(document.querySelector("#detail_image"), project.images, `${project.name} cover`);

    const steps = project.steps.map((step) => {
        const item = document.createElement("li");
        item.textContent = step;
        return item;
    });
    document.querySelector("#detail_steps").replaceChildren(...steps);

    const liveLink = document.querySelector("#detail_live_link");
    if (project.liveLink) {
        liveLink.href = project.liveLink;
        liveLink.style.display = "inline-flex";
    } else {
        liveLink.style.display = "none";
    }
}

function bindNavigation() {
    document.querySelectorAll("[data-page]").forEach((element) => {
        element.addEventListener("click", (event) => {
            const page = element.dataset.page;
            if (!page) {
                return;
            }

            event.preventDefault();
            showPage(page);
        });
    });

    menuToggle.addEventListener("click", () => {
        const isOpen = navMenu.classList.toggle("open");
        menuToggle.setAttribute("aria-expanded", String(isOpen));
    });
}

function bindGallery() {
    document.querySelector(".prev_gallery").addEventListener("click", () => moveGallery(-1));
    document.querySelector(".next_gallery").addEventListener("click", () => moveGallery(1));

    gallery.addEventListener("touchstart", (event) => {
        touchStartX = event.touches[0].clientX;
    }, { passive: true });

    gallery.addEventListener("touchend", (event) => {
        const distance = event.changedTouches[0].clientX - touchStartX;
        if (Math.abs(distance) > 42) {
            moveGallery(distance > 0 ? -1 : 1);
        }
    }, { passive: true });
}

function bindFeaturedProjectControls() {

    document
        .getElementById("featured_prev")
        .addEventListener("click", () => {
            moveFeaturedProject(-1);
        });

    document
        .getElementById("featured_next")
        .addEventListener("click", () => {
            moveFeaturedProject(1);
        });
}

function bindCopyButtons() {
    document.querySelectorAll(".copy_btn").forEach((button) => {
        button.addEventListener("click", async () => {
            const text = button.parentElement.querySelector(".copy_text").textContent.trim();

            try {
                await navigator.clipboard.writeText(text);
                button.textContent = "Copied";
            } catch (error) {
                button.textContent = "Select";
            }

            setTimeout(() => {
                button.textContent = "Copy";
            }, 1300);
        });
    });
}

async function startHeroSlideshow() {
    if (!heroImage) {
        return;
    }

    await setImageFromCandidates(heroImage, heroImages, "JD hero preview");

    setInterval(async () => {
        heroIndex = (heroIndex + 1) % heroImages.length;
        heroImage.classList.add("is_switching");
        await setImageFromCandidates(heroImage, [
            heroImages[heroIndex],
            ...heroImages.filter((_, index) => index !== heroIndex)
        ], "JD hero preview");
        heroImage.classList.remove("is_switching");
    }, 1600);
}

document.addEventListener("DOMContentLoaded", () => {
    bindNavigation();
    bindGallery();
    bindFeaturedProjectControls();
    bindCopyButtons();
    updateFeaturedProject(0);
    renderProjectList();
    startHeroSlideshow();
});
