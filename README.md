# KRON Desktop Environment

Wayland compositor (wlroots) + GTK4 shell (gtk4-layer-shell) для Arch Linux.

## Архитектура

- `kron` — Wayland compositor на wlroots (управление окнами, input, рендеринг)
- `kron-shell` — GTK4 панель и лаунчер через layer-shell протокол

## Зависимости

```bash
sudo pacman -S wlroots wayland gtk4 gtk4-layer-shell libxkbcommon meson ninja
```

## Сборка

```bash
meson setup build
ninja -C build
```

## Установка

```bash
sudo ninja -C build install
```

## Запуск

Выберите "KRON" в вашем display manager (GDM, SDDM и т.д.)

Или вручную из TTY:
```bash
kron
```

## Горячие клавиши

| Клавиша | Действие |
|---------|----------|
| Alt+Esc | Завершить сессию |
| Alt+F1  | Переключить фокус |
