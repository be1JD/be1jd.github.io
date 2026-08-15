const fallbackImage = "Assets/awakening-spirit.webp";

const heroImages = [
    "Assets/hero/rc_car_0.webp",
    "Assets/hero/rc_car_1.webp",
];

let projects = [];
let featuredProjectIndex = 0;
let selectedProjectIndex = 0;
let selectedDetailIndex = 0;
let isRoutingFromHash = false;
const prefersReducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)").matches;
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
const featuredCode = document.querySelector("#featured_code");
const featuredDemo = document.querySelector("#featured_demo");
const featuredPager = document.querySelector("#featured_pager");
const gallery = document.querySelector("#home_project_gallery");
const projectList = document.querySelector("#project_list");

async function loadProjects() {
    try {
        const response = await fetchWithTimeout("projects/projects.json");
        if (!response.ok) {
            throw new Error(`Project manifest request failed: ${response.status}`);
        }
        const manifest = await response.json();

        const results = await Promise.allSettled(manifest.map(async (project) => {
            const dataResponse = await fetchWithTimeout(`projects/${project.folder}/data.json`);
            if (!dataResponse.ok) {
                throw new Error(`Project data request failed: ${dataResponse.status}`);
            }
            const data = await dataResponse.json();
            return { ...data, folder: project.folder };
        }));

        projects = results
            .filter((result) => result.status === "fulfilled")
            .map((result) => result.value);

        const failedCount = results.length - projects.length;
        if (failedCount > 0) {
            results
                .filter((result) => result.status === "rejected")
                .forEach((result) => console.error("A project failed to load and was skipped:", result.reason));
        }

        if (!projects.length) {
            showProjectsLoadError();
            return;
        }

        updateFeaturedProject(0);
        renderProjectList();

        if (failedCount > 0) {
            showProjectsLoadWarning(failedCount);
        }
    } catch (error) {
        console.error("Failed to load projects", error);
        showProjectsLoadError();
    }
}

function fetchWithTimeout(url, timeoutMs = 10000) {
    const controller = new AbortController();
    const timer = window.setTimeout(() => controller.abort(), timeoutMs);

    return fetch(url, { signal: controller.signal }).finally(() => window.clearTimeout(timer));
}

function showProjectsLoadError() {
    if (!projectList) {
        return;
    }
    projectList.innerHTML = `
        <div class="load_error_card" role="alert">
            <p>Projects couldn't be loaded right now. This is usually a slow or interrupted
                connection, not a broken page.</p>
            <button class="ghost_link" type="button" id="retry_projects_load">Try again</button>
        </div>
    `;
    const retryButton = document.querySelector("#retry_projects_load");
    if (retryButton) {
        retryButton.addEventListener("click", () => {
            projectList.innerHTML = "";
            loadProjects();
        });
    }
}

function showProjectsLoadWarning(failedCount) {
    if (!projectList) {
        return;
    }
    const notice = document.createElement("div");
    notice.className = "load_warning_banner";
    notice.setAttribute("role", "status");
    notice.textContent = failedCount === 1
        ? "1 project couldn't be loaded and was skipped."
        : `${failedCount} projects couldn't be loaded and were skipped.`;
    projectList.prepend(notice);
}

function closeMenu() {
    navMenu.classList.remove("open");
    menuToggle.setAttribute("aria-expanded", "false");
}

function showPage(pageName, routeInfo) {
    pageViews.forEach((view) => {
        view.classList.toggle("active_page", view.dataset.view === pageName);
    });

    document.querySelectorAll(".nav_btn").forEach((button) => {
        button.classList.toggle("active_nav", button.dataset.page === pageName);
    });

    closeMenu();
    window.scrollTo({ top: 0, behavior: prefersReducedMotion ? "auto" : "smooth" });

    if (routeInfo !== null) {
        updateRoute(routeInfo || { page: pageName });
    }
}

function updateRoute(routeInfo) {
    if (isRoutingFromHash) {
        return;
    }
    const hash = routeInfo.project
        ? `#/project/${routeInfo.project}${routeInfo.tab && routeInfo.tab !== "overview" ? `/${routeInfo.tab}` : ""}`
        : `#/${routeInfo.page && routeInfo.page !== "home" ? routeInfo.page : ""}`;

    if (window.location.hash !== hash) {
        window.history.pushState(null, "", hash || "#/");
    }
}

function parseRoute() {
    const raw = window.location.hash.replace(/^#\/?/, "");
    const parts = raw.split("/").filter(Boolean);

    if (!parts.length) {
        return { page: "home" };
    }

    if (parts[0] === "project" && parts[1]) {
        return { page: "project-detail", project: parts[1], tab: parts[2] || "overview" };
    }

    return { page: parts[0] };
}

async function applyRoute() {
    const route = parseRoute();
    isRoutingFromHash = true;

    try {
        if (route.page === "project-detail" && route.project) {
            const project = projects.find((item) => item.folder === route.project);
            if (project) {
                await openProject(project, route.tab || "overview", null);
            } else {
                showPage("home", null);
            }
        } else {
            const validPages = ["home", "projects", "about", "contact"];
            showPage(validPages.includes(route.page) ? route.page : "home", null);
        }
    } finally {
        isRoutingFromHash = false;
    }
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

async function openProject(project, target = "overview", pushHistory = true) {
    try {
        await renderProjectDetail(project);
        showPage("project-detail", pushHistory === false ? null : { project: project.folder, tab: target });
        const resolvedTarget = target === "code" && !project.codeHref ? "overview" : target;
        setProjectTab(resolvedTarget);
        window.scrollTo({ top: 0, behavior: "auto" });
        if (resolvedTarget === "code") {
            window.requestAnimationFrame(scrollToFirmwareCode);
        }
    } catch (error) {
        console.error("Failed to open project detail", error);
    }
}

function setProjectTab(mode) {
    const tabs = document.querySelectorAll("#detail_tabs [data-project-mode]");
    if (!tabs.length) {
        return;
    }

    let resolvedMode = mode;
    const targetTab = document.querySelector(`#detail_tabs [data-project-mode="${mode}"]`);
    if (!targetTab || targetTab.style.display === "none") {
        resolvedMode = "overview";
    }

    tabs.forEach((tab) => {
        const isActive = tab.dataset.projectMode === resolvedMode;
        tab.classList.toggle("active", isActive);
        tab.setAttribute("aria-selected", String(isActive));
    });

    document.querySelectorAll("[data-project-view]").forEach((view) => {
        view.classList.toggle("active_project_mode", view.dataset.projectView === resolvedMode);
    });
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
    setLinkState(featuredDemo, project.demoLink || project.liveLink, "▶ View Demo");

    featuredOpen.onclick = () => openProject(project);
    if (featuredCode) {
        featuredCode.style.display = project.codeHref ? "inline-flex" : "none";
        featuredCode.onclick = () => openProject(project, "code");
    }
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
    if (prefersReducedMotion) {
        return;
    }
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

    const tagBadge = document.createElement("span");
    tagBadge.className = "project_list_tag";
    tagBadge.textContent = project.tag;

    const progressBadge = document.createElement("span");
    progressBadge.className = "project_list_progress";
    progressBadge.textContent = `${project.progress}% built`;

    thumbWrap.append(image, tagBadge, progressBadge);

    const body = document.createElement("div");
    body.className = "project_list_body";

    const meta = document.createElement("p");
    meta.className = "project_meta";
    meta.textContent = `${project.tag} · ${project.progress}% complete`;

    const title = document.createElement("h2");
    title.textContent = project.name;

    const description = document.createElement("p");
    description.textContent = project.description;

    const actions = document.createElement("div");
    actions.className = "project_list_actions";

    if (project.demoLink || project.liveLink) {
        const demo = document.createElement("a");
        demo.className = "primary_btn";
        demo.href = project.demoLink || project.liveLink;
        demo.target = "_blank";
        demo.rel = "noreferrer";
        demo.textContent = "▶ View Demo";
        actions.append(demo);
    }

    if (project.codeHref) {
        const codeButton = document.createElement("button");
        codeButton.className = "code_btn";
        codeButton.type = "button";
        codeButton.textContent = "</> Get Code";
        codeButton.addEventListener("click", (event) => {
            event.preventDefault();
            event.stopPropagation();
            openProject(project, "code");
        });
        actions.append(codeButton);
    }

    const guideButton = document.createElement("button");
    guideButton.className = "guide_btn";
    guideButton.type = "button";
    guideButton.textContent = "Full Guide";
    guideButton.addEventListener("click", (event) => {
        event.preventDefault();
        event.stopPropagation();
        openProject(project);
    });
    actions.append(guideButton);

    body.append(title, meta, description, actions);
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
        const response = await fetchWithTimeout(project.guideHref);
        if (!response.ok) {
            throw new Error(`Guide request failed: ${response.status}`);
        }
        return await response.json();
    } catch (error) {
        console.error("Failed to load project guide", error);
        return null;
    }
}

function highlightFirmwareCode(codeElement, codeHref = "") {
    const extension = (codeHref.split(".").pop() || "").toLowerCase();
    const languageMap = { ino: "cpp", cpp: "cpp", cc: "cpp", h: "cpp", hpp: "cpp", c: "c", py: "python", js: "javascript" };
    const language = languageMap[extension] || "cpp";

    codeElement.className = `language-${language}`;

    const filenameLabel = document.querySelector("#code_window_filename");
    if (filenameLabel) {
        const filename = codeHref.split("/").pop() || "firmware";
        filenameLabel.textContent = filename;
    }

    if (window.Prism && window.Prism.highlightElement) {
        window.Prism.highlightElement(codeElement);
    }
}

async function loadProjectCode(project) {
    if (!project.codeHref) {
        return "";
    }

    try {
        const response = await fetchWithTimeout(project.codeHref);
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
    selectedDetailIndex = projects.indexOf(project);
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
    const codeTabButton = document.querySelector("#tab_btn_code");
    const quickCodeButton = document.querySelector("#quick_code_button");

    const hasCode = Boolean(code && codeElement);

    if (hasCode) {
        codeElement.textContent = code;
        highlightFirmwareCode(codeElement, project.codeHref);
    } else if (codeElement) {
        codeElement.textContent = "";
    }

    if (codeSection) {
        codeSection.style.display = hasCode ? "grid" : "none";
    }
    if (codeButton) {
        codeButton.style.display = hasCode ? "inline-flex" : "none";
    }
    if (codeTabButton) {
        codeTabButton.style.display = hasCode ? "inline-flex" : "none";
    }
    if (quickCodeButton) {
        quickCodeButton.style.display = hasCode ? "inline-flex" : "none";
    }

    if (hasCode) {
        [codeButton, quickCodeButton].forEach((button) => {
            if (!button) return;
            const clone = button.cloneNode(true);
            button.replaceWith(clone);
        });
        document.querySelectorAll("#detail_code_button, #quick_code_button").forEach((button) => {
            button.addEventListener("click", (event) => {
                event.preventDefault();
                event.stopPropagation();
                setProjectTab("code");
                updateRoute({ project: project.folder, tab: "code" });
                window.requestAnimationFrame(scrollToFirmwareCode);
            });
        });

        const copyClone = copyButton.cloneNode(true);
        copyButton.replaceWith(copyClone);
        copyClone.addEventListener("click", async () => {
            try {
                await navigator.clipboard.writeText(code);
                copyClone.textContent = "Copied to clipboard";
                copyClone.classList.add("copied");
            } catch (error) {
                copyClone.textContent = "Select code manually";
            }
            window.setTimeout(() => {
                copyClone.textContent = "Copy code";
                copyClone.classList.remove("copied");
            }, 1600);
        });
    }

    setLinkState(document.querySelector("#detail_demo_link"), project.demoLink || project.liveLink, "▶ Open Demo");
    setLinkState(document.querySelector("#detail_live_link"), project.liveLink, "Project Page");

    const quickTitle = document.querySelector("#quick_action_title");
    if (quickTitle) {
        quickTitle.textContent = project.name;
    }
    setLinkState(document.querySelector("#quick_demo_link"), project.demoLink || project.liveLink, "▶ Demo");

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
            setProjectTab(button.dataset.projectMode);
            const current = projects[selectedDetailIndex];
            if (current) {
                updateRoute({ project: current.folder, tab: button.dataset.projectMode });
            }
            if (button.dataset.projectMode === "code") {
                window.requestAnimationFrame(scrollToFirmwareCode);
            }
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

function initThemeToggle() {
    const toggleButton = document.querySelector("#theme_toggle");
    if (!toggleButton) {
        return;
    }

    const storedTheme = window.localStorage.getItem("be1jd_theme");
    const systemPrefersLight = window.matchMedia("(prefers-color-scheme: light)").matches;
    const initialTheme = storedTheme || (systemPrefersLight ? "light" : "dark");

    applyTheme(initialTheme, toggleButton);

    toggleButton.addEventListener("click", () => {
        const currentTheme = document.documentElement.dataset.theme === "light" ? "light" : "dark";
        const nextTheme = currentTheme === "light" ? "dark" : "light";
        applyTheme(nextTheme, toggleButton);
        window.localStorage.setItem("be1jd_theme", nextTheme);
    });
}

function applyTheme(theme, toggleButton) {
    if (theme === "light") {
        document.documentElement.setAttribute("data-theme", "light");
        toggleButton.setAttribute("aria-pressed", "true");
        toggleButton.setAttribute("aria-label", "Switch to dark mode");
    } else {
        document.documentElement.removeAttribute("data-theme");
        toggleButton.setAttribute("aria-pressed", "false");
        toggleButton.setAttribute("aria-label", "Switch to light mode");
    }
}

function hideSiteLoader() {
    const loader = document.querySelector("#site_loader");
    if (!loader || loader.classList.contains("site_loader_hidden")) {
        return;
    }
    loader.classList.add("site_loader_hidden");
    window.setTimeout(() => loader.remove(), 500);
}

// Safety net: never let a slow or failed request leave the loading screen
// stuck on top of the page. This fires regardless of what else happens below.
window.setTimeout(hideSiteLoader, 6000);
window.addEventListener("error", hideSiteLoader);
window.addEventListener("unhandledrejection", hideSiteLoader);

document.addEventListener("DOMContentLoaded", async () => {
    try {
        initThemeToggle();
        bindNavigation();
        bindGallery();
        bindFeaturedProjectControls();
        bindProjectViewToggle();
        bindCopyButtons();
        await loadProjects();
        await applyRoute();
        window.addEventListener("hashchange", applyRoute);
        if (!prefersReducedMotion) {
            startHeroSlideshow();
        }
    } catch (error) {
        console.error("Startup encountered an error, showing the site anyway:", error);
    } finally {
        hideSiteLoader();
    }
});
