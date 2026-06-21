const fallbackImage = "Assets/awakening-spirit.webp";

const heroImages = [
    "Assets/hero/gallery1.png",
    fallbackImage,
];

let projects = [];
let featuredProjectIndex = 0;
let selectedProjectIndex = 0;
let activeGalleryIndex = 0;
let selectorImageIndex = 0;
let detailImageIndex = 0;
let heroIndex = 0;
let touchStartX = 0;
let galleryTimer = 0;

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
const featuredDemo = document.querySelector("#featured_demo");
const featuredPager = document.querySelector("#featured_pager");
const gallery = document.querySelector("#home_project_gallery");
const projectQueue = document.querySelector("#project_queue");
const projectCards = document.querySelector("#project_cards");

async function loadProjects() {
    try {
        const response = await fetch("projects/projects.json");
        const manifest = await response.json();

        projects = await Promise.all(manifest.map(async (project) => {
            const dataResponse = await fetch(`projects/${project.folder}/data.json`);
            const data = await dataResponse.json();
            return { ...data, folder: project.folder };
        }));

        updateFeaturedProject(0);
        renderProjectModes();
        updateSelectorProject(0);
    } catch (error) {
        console.error("Failed to load projects", error);
    }
}

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
    if (!imageElement) {
        return "";
    }

    const sources = [...new Set([...(candidates || []), fallbackImage])];
    imageElement.classList.add("image_switching");

    for (const source of sources) {
        const loaded = await imageExists(source);
        if (loaded) {
            imageElement.src = loaded;
            imageElement.alt = altText;
            imageElement.classList.remove("image_switching");
            return loaded;
        }
    }

    imageElement.classList.remove("image_switching");
    return "";
}

function setProgress(element, progress = 0) {
    element.style.setProperty("--progress", progress);
    element.querySelector("span").textContent = `${progress}%`;
}

function setLinkState(link, url, label) {
    if (!link) {
        return;
    }

    if (url) {
        link.href = url;
        link.textContent = label;
        link.style.display = "inline-flex";
    } else {
        link.style.display = "none";
    }
}

function renderTags(container, items = []) {
    container.replaceChildren(...items.map((item) => {
        const tag = document.createElement("span");
        tag.textContent = item;
        return tag;
    }));
}

function renderResources(container, resources = []) {
    container.replaceChildren(...resources.map((resource) => {
        const link = document.createElement("a");
        link.href = resource.href;
        link.textContent = resource.label;
        link.target = "_blank";
        link.rel = "noreferrer";
        return link;
    }));
}

function openProject(project) {
    renderProjectDetail(project);
    showPage("project-detail");
}

function updateFeaturedProject(index) {
    if (!projects.length) {
        return;
    }

    featuredProjectIndex = (index + projects.length) % projects.length;
    activeGalleryIndex = 0;

    const project = projects[featuredProjectIndex];
    featuredMeta.textContent = project.tag;
    featuredTitle.textContent = project.name;
    featuredDescription.textContent = project.description;
    setProgress(featuredProgress, project.progress);
    renderTags(featuredFeatures, project.features);
    setLinkState(featuredDemo, project.demoLink || project.liveLink, "Demo");

    featuredOpen.onclick = () => openProject(project);
    renderFeaturedPager();
    updateFeaturedImage();
    restartGalleryTimer();
}

function updateFeaturedImage() {
    const project = projects[featuredProjectIndex];
    const orderedImages = [
        project.images[activeGalleryIndex],
        ...project.images.filter((_, index) => index !== activeGalleryIndex)
    ];

    setImageFromCandidates(featuredImage, orderedImages, `${project.name} preview`);
}

function moveGallery(direction) {
    if (!projects.length) {
        return;
    }

    const imageCount = projects[featuredProjectIndex].images.length;
    activeGalleryIndex = (activeGalleryIndex + direction + imageCount) % imageCount;
    updateFeaturedImage();
    restartGalleryTimer();
}

function restartGalleryTimer() {
    window.clearInterval(galleryTimer);
    galleryTimer = window.setInterval(() => {
        if (projects.length) {
            const imageCount = projects[featuredProjectIndex].images.length;
            activeGalleryIndex = (activeGalleryIndex + 1) % imageCount;
            updateFeaturedImage();
        }
    }, 1800);
}

function moveFeaturedProject(direction) {
    updateFeaturedProject(featuredProjectIndex + direction);
}

function renderFeaturedPager() {
    featuredPager.replaceChildren(...projects.map((project, index) => {
        const button = document.createElement("button");
        button.type = "button";
        button.className = index === featuredProjectIndex ? "active" : "";
        button.setAttribute("aria-label", `Show ${project.name}`);
        button.addEventListener("click", () => updateFeaturedProject(index));
        return button;
    }));
}

function createProjectQueueItem(project, index) {
    const button = document.createElement("button");
    button.className = `queue_item${index === selectedProjectIndex ? " active" : ""}`;
    button.type = "button";
    button.addEventListener("click", () => updateSelectorProject(index));

    const image = document.createElement("img");
    image.alt = `${project.name} thumbnail`;
    setImageFromCandidates(image, project.images, `${project.name} thumbnail`);

    const meta = document.createElement("p");
    meta.className = "project_meta";
    meta.textContent = `${project.tag} / ${project.progress}%`;

    const title = document.createElement("h3");
    title.textContent = project.name;

    button.append(image, meta, title);
    return button;
}

function createProjectCard(project) {
    const card = document.createElement("article");
    card.className = "project_card";

    const image = document.createElement("img");
    image.alt = `${project.name} thumbnail`;
    setImageFromCandidates(image, project.images, `${project.name} thumbnail`);

    const meta = document.createElement("p");
    meta.className = "project_meta";
    meta.textContent = project.tag;

    const title = document.createElement("h2");
    title.textContent = project.name;

    const description = document.createElement("p");
    description.textContent = project.description;

    const button = document.createElement("button");
    button.className = "primary_btn";
    button.type = "button";
    button.textContent = "Open Guide";
    button.addEventListener("click", () => openProject(project));

    card.append(image, meta, title, description, button);
    return card;
}

function renderProjectModes() {
    projectQueue.replaceChildren(...projects.map(createProjectQueueItem));
    projectCards.replaceChildren(...projects.map(createProjectCard));
}

function renderThumbs(container, project, selectedIndex, onSelect) {
    container.replaceChildren(...project.images.map((src, index) => {
        const button = document.createElement("button");
        button.type = "button";
        button.className = `thumb_btn${index === selectedIndex ? " active" : ""}`;
        button.setAttribute("aria-label", `Show image ${index + 1}`);
        button.addEventListener("click", () => onSelect(index));

        const image = document.createElement("img");
        image.src = src;
        image.alt = "";
        button.append(image);
        return button;
    }));
}

function updateSelectorProject(index) {
    if (!projects.length) {
        return;
    }

    selectedProjectIndex = (index + projects.length) % projects.length;
    selectorImageIndex = 0;
    const project = projects[selectedProjectIndex];

    document.querySelector("#selector_meta").textContent = `${project.tag} / ${project.progress}% complete`;
    document.querySelector("#selector_title").textContent = project.name;
    document.querySelector("#selector_description").textContent = project.description;
    renderTags(document.querySelector("#selector_features"), project.features);
    renderResources(document.querySelector("#selector_resources"), project.resources);
    setLinkState(document.querySelector("#selector_demo"), project.demoLink || project.liveLink, "Demo");
    document.querySelector("#selector_open").onclick = () => openProject(project);

    const renderSelectorThumbs = () => {
        renderThumbs(document.querySelector("#selector_thumbs"), project, selectorImageIndex, (thumbIndex) => {
            selectorImageIndex = thumbIndex;
            setImageFromCandidates(document.querySelector("#selector_image"), [
                project.images[selectorImageIndex],
                ...project.images
            ], `${project.name} preview`);
            renderSelectorThumbs();
        });
    };

    setImageFromCandidates(document.querySelector("#selector_image"), project.images, `${project.name} preview`);
    renderSelectorThumbs();

    document.querySelectorAll(".queue_item").forEach((item, itemIndex) => {
        item.classList.toggle("active", itemIndex === selectedProjectIndex);
    });
}

function updateDetailImage(project, thumbIndex) {
    detailImageIndex = thumbIndex;
    setImageFromCandidates(document.querySelector("#detail_image"), [
        project.images[detailImageIndex],
        ...project.images
    ], `${project.name} cover`);
}

function renderDetailThumbs(project) {
    renderThumbs(document.querySelector("#detail_thumbs"), project, detailImageIndex, (thumbIndex) => {
        updateDetailImage(project, thumbIndex);
        renderDetailThumbs(project);
    });
}

function renderProjectDetail(project) {
    detailImageIndex = 0;
    document.querySelector("#detail_meta").textContent = `${project.tag} / ${project.folder}`;
    document.querySelector("#detail_title").textContent = project.name;
    document.querySelector("#detail_description").textContent = project.description;
    document.querySelector("#detail_overview").textContent = project.overview;
    document.querySelector("#detail_price").textContent = project.price || "Open resources";
    setProgress(document.querySelector("#detail_progress"), project.progress);
    setImageFromCandidates(document.querySelector("#detail_image"), project.images, `${project.name} cover`);
    renderResources(document.querySelector("#detail_resources"), project.resources);

    const steps = (project.steps || []).map((step) => {
        const item = document.createElement("li");
        item.textContent = step;
        return item;
    });
    document.querySelector("#detail_steps").replaceChildren(...steps);

    setLinkState(document.querySelector("#detail_demo_link"), project.demoLink || project.liveLink, "Open Demo");
    setLinkState(document.querySelector("#detail_live_link"), project.liveLink, "Project Page");

    renderDetailThumbs(project);
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
    document.getElementById("featured_prev").addEventListener("click", () => moveFeaturedProject(-1));
    document.getElementById("featured_next").addEventListener("click", () => moveFeaturedProject(1));
}

function bindProjectViewToggle() {
    document.querySelectorAll("[data-project-mode]").forEach((button) => {
        button.addEventListener("click", () => {
            const mode = button.dataset.projectMode;
            document.querySelectorAll("[data-project-mode]").forEach((toggle) => {
                toggle.classList.toggle("active", toggle === button);
            });
            document.querySelectorAll("[data-project-view]").forEach((view) => {
                view.classList.toggle("active_project_mode", view.dataset.projectView === mode);
            });
        });
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
    }, 2200);
}

document.addEventListener("DOMContentLoaded", async () => {
    bindNavigation();
    bindGallery();
    bindFeaturedProjectControls();
    bindProjectViewToggle();
    bindCopyButtons();
    await loadProjects();
    startHeroSlideshow();
});
