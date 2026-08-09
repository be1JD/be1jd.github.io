const fallbackImage = "Assets/awakening-spirit.webp";

const heroImages = [
    "Assets/hero/rc_car_0.webp",
    "Assets/hero/rc_car_1.webp",
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
const projectList = document.querySelector("#project_list");

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
        renderProjectList();
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

async function openProject(project, target = "guide") {
    try {
        await renderProjectDetail(project);
        showPage("project-detail");

        if (target === "code") {
            window.requestAnimationFrame(scrollToFirmwareCode);
        }
    } catch (error) {
        console.error("Failed to open project detail", error);
    }
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

function createProjectListItem(project) {
    const row = document.createElement("article");
    row.className = "project_list_item";
    row.addEventListener("click", (event) => {
        const target = event.target instanceof Element ? event.target.closest("a, button") : null;
        if (!target) {
            openProject(project);
        }
    });

    const thumbWrap = document.createElement("div");
    thumbWrap.className = "project_list_thumb";

    const image = document.createElement("img");
    image.alt = `${project.name} thumbnail`;
    setImageFromCandidates(image, project.images, `${project.name} thumbnail`);
    thumbWrap.append(image);

    const body = document.createElement("div");
    body.className = "project_list_body";

    const meta = document.createElement("p");
    meta.className = "project_meta";
    meta.textContent = `${project.tag} / ${project.progress}% complete`;

    const title = document.createElement("h2");
    title.textContent = project.name;

    const description = document.createElement("p");
    description.textContent = project.description;

    const actions = document.createElement("div");
    actions.className = "project_list_actions";

    const guideButton = document.createElement("button");
    guideButton.className = "primary_btn";
    guideButton.type = "button";
    guideButton.textContent = "Open Full Guide";
    guideButton.addEventListener("click", (event) => {
        event.preventDefault();
        event.stopPropagation();
        openProject(project);
    });
    actions.append(guideButton);

    if (project.codeHref) {
        const codeButton = document.createElement("button");
        codeButton.className = "ghost_link";
        codeButton.type = "button";
        codeButton.textContent = "Code";
        codeButton.addEventListener("click", (event) => {
            event.preventDefault();
            event.stopPropagation();
            openProject(project, "code");
        });
        actions.append(codeButton);
    }

    if (project.demoLink || project.liveLink) {
        const demo = document.createElement("a");
        demo.className = "ghost_link";
        demo.href = project.demoLink || project.liveLink;
        demo.target = "_blank";
        demo.rel = "noreferrer";
        demo.textContent = "Demo";
        actions.append(demo);
    }

    body.append(meta, title, description, actions);
    row.append(thumbWrap, body);
    return row;
}

function renderProjectList() {
    if (!projectList) {
        return;
    }

    projectList.replaceChildren(...projects.map(createProjectListItem));
}

async function loadProjectGuide(project) {
    if (!project.guideHref) {
        return null;
    }

    try {
        const response = await fetch(project.guideHref);
        if (!response.ok) {
            throw new Error(`Guide request failed: ${response.status}`);
        }
        return await response.json();
    } catch (error) {
        console.error("Failed to load project guide", error);
        return null;
    }
}

async function loadProjectCode(project) {
    if (!project.codeHref) {
        return "";
    }

    try {
        const response = await fetch(project.codeHref);
        if (!response.ok) {
            throw new Error(`Code request failed: ${response.status}`);
        }
        return await response.text();
    } catch (error) {
        console.error("Failed to load project code", error);
        return "";
    }
}

function createGuideGroup(title, items, ordered = false) {
    const group = document.createElement("section");
    group.className = "guide_doc_group";

    const heading = document.createElement("h3");
    heading.textContent = title;
    group.append(heading);

    if (!items || !items.length) {
        return group;
    }

    if (items.length === 1) {
        const paragraph = document.createElement("p");
        paragraph.textContent = items[0];
        group.append(paragraph);
        return group;
    }

    const list = document.createElement(ordered ? "ol" : "ul");
    items.forEach((text) => {
        const item = document.createElement("li");
        item.textContent = text;
        list.append(item);
    });
    group.append(list);
    return group;
}

function renderGuideContent(guide) {
    const container = document.querySelector("#detail_guide_content");
    const docSection = document.querySelector("#detail_doc_section");

    if (!container || !docSection) {
        return;
    }

    if (!guide) {
        docSection.style.display = "none";
        container.replaceChildren();
        return;
    }

    docSection.style.display = "grid";
    const blocks = [];
    blocks.push(createGuideGroup("Introduction", guide.intro || []));

    (guide.highlights || []).forEach((group) => {
        blocks.push(createGuideGroup(group.title, group.items || []));
    });

    blocks.push(createGuideGroup("Supplies", guide.supplies || []));

    (guide.sections || []).forEach((section) => {
        blocks.push(createGuideGroup(section.title, section.items || [], true));
    });

    container.replaceChildren(...blocks);
}

function scrollToFirmwareCode() {
    const section = document.querySelector("#firmware_code_section");
    if (section && section.style.display !== "none") {
        section.scrollIntoView({ behavior: "smooth", block: "start" });
    }
}
function updateDetailImage(project, thumbIndex) {
    detailImageIndex = thumbIndex;
    setImageFromCandidates(document.querySelector("#detail_image"), [
        project.images[detailImageIndex],
        ...project.images
    ], `${project.name} cover`);
}

function renderThumbs(container, project, activeIndex, onSelect) {
    if (!container || !project || !Array.isArray(project.images)) {
        return;
    }

    const thumbs = project.images.map((source, index) => {
        const button = document.createElement("button");
        button.type = "button";
        button.className = `thumb_btn${index === activeIndex ? " active" : ""}`;
        button.addEventListener("click", () => onSelect(index));

        const image = document.createElement("img");
        image.src = source;
        image.alt = `${project.name} thumbnail ${index + 1}`;
        button.append(image);
        return button;
    });

    container.replaceChildren(...thumbs);
}

function renderDetailThumbs(project) {
    renderThumbs(document.querySelector("#detail_thumbs"), project, detailImageIndex, (thumbIndex) => {
        updateDetailImage(project, thumbIndex);
        renderDetailThumbs(project);
    });
}

async function renderProjectDetail(project) {
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

    const [guide, code] = await Promise.all([
        loadProjectGuide(project),
        loadProjectCode(project)
    ]);
    renderGuideContent(guide);

    const codeSection = document.querySelector("#firmware_code_section");
    const codeElement = document.querySelector("#firmware_code");
    const copyButton = document.querySelector("#copy_firmware_code");
    const codeButton = document.querySelector("#detail_code_button");

    if (code && codeElement && codeSection && codeButton) {
        codeElement.textContent = code;
        codeSection.style.display = "grid";
        codeButton.style.display = "inline-flex";
        codeButton.replaceWith(codeButton.cloneNode(true));
        const newCodeButton = document.querySelector("#detail_code_button");
        if (newCodeButton) {
            newCodeButton.addEventListener("click", (event) => {
                event.preventDefault();
                event.stopPropagation();
                openProject(project, "code");
            });
        }

        copyButton.replaceWith(copyButton.cloneNode(true));
        const newCopyButton = document.querySelector("#copy_firmware_code");
        if (newCopyButton) {
            newCopyButton.addEventListener("click", async () => {
                try {
                    await navigator.clipboard.writeText(code);
                    newCopyButton.textContent = "Copied";
                } catch (error) {
                    newCopyButton.textContent = "Select";
                }
                window.setTimeout(() => {
                    newCopyButton.textContent = "Copy";
                }, 1300);
            });
        }
    } else {
        if (codeElement) {
            codeElement.textContent = "";
        }
        if (codeSection) {
            codeSection.style.display = "none";
        }
        if (codeButton) {
            codeButton.style.display = "none";
        }
    }

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

async function resolveImageSource(candidates) {
    const sources = [...new Set([...(candidates || []), fallbackImage])];

    for (const source of sources) {
        const loaded = await imageExists(source);
        if (loaded) {
            return loaded;
        }
    }

    return "";
}

function wait(ms) {
    return new Promise((resolve) => window.setTimeout(resolve, ms));
}

async function startHeroSlideshow() {
    if (!heroImage) {
        return;
    }

    const holdDuration = 2200;
    const transitionDuration = 1000;
    const heroStage = heroImage.parentElement;
    const firstImage = await resolveImageSource(heroImages);

    if (!firstImage || !heroStage) {
        return;
    }

    heroImage.src = firstImage;
    heroImage.alt = "JD hero preview";
    heroImage.classList.add("active_hero_image");

    while (heroImages.length > 1) {
        await wait(holdDuration);

        heroIndex = (heroIndex + 1) % heroImages.length;
        const nextSource = await resolveImageSource([
            heroImages[heroIndex],
            ...heroImages.filter((_, index) => index !== heroIndex)
        ]);

        const nextSourceUrl = new URL(nextSource, window.location.href).href;
        if (!nextSource || nextSourceUrl === heroImage.currentSrc) {
            continue;
        }

        const nextImage = heroImage.cloneNode(false);
        nextImage.src = nextSource;
        nextImage.alt = "JD hero preview";
        nextImage.classList.remove("active_hero_image", "leaving_hero_image", "image_switching");
        heroStage.insertBefore(nextImage, heroImage.nextSibling);

        window.requestAnimationFrame(() => {
            heroImage.classList.add("leaving_hero_image");
            heroImage.classList.remove("active_hero_image");
            nextImage.classList.add("active_hero_image");
        });

        await wait(transitionDuration);
        heroImage.src = nextSource;
        heroImage.classList.remove("leaving_hero_image");
        heroImage.classList.add("active_hero_image");
        nextImage.remove();
    }
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
