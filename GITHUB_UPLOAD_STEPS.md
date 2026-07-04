# GitHub Upload Steps

Repository: `https://github.com/AydinEZP/ProteusClone.git`

Authors used in the generated local history:

- Aydin `<AydinEZP@gmail.com>` — UI / Editor / Project Management
- Sepehr `<Sepehrfazli77@gmail.com>` — Circuit Core / Simulation
- Mohsen `<mohsensharifi787980@gmail.com>` — Advanced Components

Run from the project root:

```bash
bash scripts/create_commit_history.sh
git log --oneline --decorate --all
git push -u origin main
git push origin v1.0-final
```

Do not create any file from GitHub web UI before running this, because the repository is currently empty and this script is intended to create the first history cleanly.
