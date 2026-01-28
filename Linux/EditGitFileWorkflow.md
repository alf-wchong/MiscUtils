# Fedora CLI on WSL2: GitHub File Edits Workflow

**Step-by-step instructions to clone from `main`, create a branch, edit files, commit/push changes, and create a PR using Fedora CLI on WSL2.** \
Use SSH for GitHub access. Replace placeholders: `<repo-url>` (e.g., `git@github.com:owner/repo.git`), `<branch-name>` (e.g., `my-feature`), `<file-paths>` (e.g., `src/main.py`), `<commit-message>`, `<PR-title>`.

This guide assumes Git, SSH keys added to GitHub, and basic familiarity. Tested on Fedora WSL2.[^1][^2][^3]

## Prerequisites

- **Verify SSH**: `ssh -T git@github.com` (expect "Hi <username>!").
- **SSH Agent (if needed)**:

```
eval $(ssh-agent -s)
ssh-add ~/.ssh/<private-key>  # e.g., id_ed25519
```

For persistent: `systemctl --user enable --now ssh-agent` (systemd socket).[^4][^5]
- **GitHub CLI (gh)**: `sudo dnf install gh` then `gh auth login` (SSH mode).[^6][^7]
- **Git**: Usually pre-installed; `git --version`.


## 1. Start SSH Agent

```
eval $(ssh-agent -s)
ssh-add ~/.ssh/<private-key>
ssh -T git@github.com  # Confirm auth
```


## 2. Clone from Main Branch

```
git clone <repo-url> <local-dir>  # e.g., my-repo
cd <local-dir>
git checkout main
git pull origin main  # Ensure latest
```


## 3. Create Local Branch from Main

```
git checkout -b <branch-name>  # Or: git switch -c <branch-name>
git branch  # Verify: * <branch-name>
```


## 4. Make Local Changes

- Edit: `vim <file-paths>` or `nano <file-paths>`.
- Check: `git status` and `git diff <file-paths>`.


## 5. Commit and Push Changes

```
git add <file-paths>  # Or: git add .
git commit -m "<commit-message>"
git push -u origin <branch-name>  # Sets upstream
```


## 6. Create Pull Request (PR)

```
gh pr create --title "<PR-title>" --body "<description>" --base main
```

- Opens PR link in terminal/browser.
- Review/merge on GitHub web.


## Troubleshooting

| Issue | Command/Solution |
| :-- | :-- |
| SSH "Permission denied" | `ssh-add -l`; re-add key, check `~/.ssh/config`. [^8] |
| "Repository not found" | Verify `<repo-url>`, access rights. [^9] |
| No gh | `sudo dnf install gh`. [^10] |
| WSL2 networking | Restart WSL: `wsl --shutdown` (PowerShell). [^11] |

**Save as `github-edits-workflow.md` and commit to your repo root or `/docs` for GitHub rendering.**[^12][^1]
<span style="display:none">[^13][^14][^15][^16][^17][^18][^19][^20][^21][^22][^23][^24][^25]</span>

<div align="center">⁂</div>

[^1]: https://docs.github.com/github/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax

[^2]: https://discussion.fedoraproject.org/t/auto-add-keys-to-ssh-agent-at-login/117057

[^3]: https://github.com/cli/cli/discussions/2863

[^4]: https://linux-audit.com/ssh/faq/how-to-start-ssh-agent/

[^5]: https://www.tomica.net/blog/2016/10/ssh-agent-service-in-fedora/

[^6]: https://github.com/cli/cli/blob/trunk/docs/install_linux.md

[^7]: https://docs.github.com/en/github-cli/github-cli/quickstart

[^8]: https://www.perplexity.ai/search/477493d7-cfe1-4e89-8a99-fc2f7c184666

[^9]: https://www.perplexity.ai/search/c33d47a0-54bf-4480-8d2f-408f1281499b

[^10]: https://www.amankumarsingh.me/blog/667069e5910e676715a42069

[^11]: https://www.perplexity.ai/search/5831da1d-e4d0-4da0-831e-cd32a3f21b18

[^12]: https://docs.github.com/en/repositories/managing-your-repositorys-settings-and-features/customizing-your-repository/about-readmes

[^13]: https://github.com/darsaveli/Readme-Markdown-Syntax

[^14]: https://www.reddit.com/r/webdev/comments/18sozpf/how_do_you_write_your_readmemd_or_docs_for_your/

[^15]: https://www.geeksforgeeks.org/git/what-is-github-readme-file-and-markdown/

[^16]: https://www.reddit.com/r/Markdown/comments/1pw5u45/i_made_a_cli_to_convert_markdown_to_githubstyled/

[^17]: https://www.youtube.com/watch?v=s_MV82dy0jY

[^18]: https://stackoverflow.com/questions/7694887/is-there-a-command-line-utility-for-rendering-github-flavored-markdown

[^19]: https://github.com/orgs/community/discussions/22833

[^20]: https://github.com/Textualize/rich-cli

[^21]: https://github.com/adam-p/markdown-here/wiki/markdown-cheatsheet

[^22]: https://nicolas-van.github.io/easy-markdown-to-github-pages/

[^23]: https://github.com/StructuredLabs/preswald/issues/647

[^24]: https://github.com/solworktech/md2pdf

[^25]: https://github.com/typora/typora-issues/issues/3839

