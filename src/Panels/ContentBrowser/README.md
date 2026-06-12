# Panels/ContentBrowser

Asset browser with async thumbnails.

- **ContentBrowserPanel** — grid view over the `assets/` tree: navigation, asset import, drag-and-drop sources for the viewport/inspector.
- **Directory** — cached filesystem tree the panel browses.
- **AssetPreviewManager** — renders asset thumbnails asynchronously via the **Preview** interface: **Texture2DPreview**, **MaterialPreview** (lit sphere), **PrefabPreview** (offscreen scene render).
- **EditorIcons** — built-in icons for non-previewable file types.
- **LoadedAssets** — tracks which assets the browser has live previews/handles for.
