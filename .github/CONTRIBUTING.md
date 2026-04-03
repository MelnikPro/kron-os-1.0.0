# Contributing to KRON

Thank you for your interest in contributing to KRON Desktop Environment.

## Getting Started

1. Fork the repository on GitHub
2. Clone your fork locally
3. Set up the development environment (see README.md)
4. Make your changes
5. Test your changes
6. Submit a pull request

## Code Style

- C11 standard
- 4-space indentation
- Keep functions small and focused
- Comment non-obvious code
- Follow existing naming conventions (`kron_module_action`)

## Reporting Bugs

Open an issue with:
- Arch Linux version (`uname -a`)
- wlroots version (`pkg-config --modversion wlroots-0.18`)
- Steps to reproduce
- Expected vs actual behavior
- Relevant log output (`kron 2>&1 | tee kron.log`)

## Feature Requests

Open an issue with the `enhancement` label describing:
- What problem it solves
- Proposed implementation approach
