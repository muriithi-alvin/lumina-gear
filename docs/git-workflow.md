# Lumina Gear: Team Git Guide

**Project:** Lumina Gear, a 2D Mechanical Power and Illumination Simulator
**Team:** 6 members, working in 3 pairs
**Project Lead:** manages the repository and merges finished checkpoints

This guide shows every Git step you need. Follow it exactly and we will avoid most problems.

---

## 1. The Big Picture

Each checkpoint has its **own branch**. The pair that owns the checkpoint does the work on small **task branches** and merges them into the checkpoint branch. The Project Lead then merges the finished checkpoint into `main`.

```
task branch  -->  checkpoint branch  -->  main (tagged)
(your pair)        (your pair)            (Project Lead)
```

- `main` holds only finished, working versions. **Nobody commits to it directly.**
- A checkpoint branch holds the work for one checkpoint.
- A task branch holds one small job done by one person.

---

## 2. Branches

| Checkpoint | Checkpoint branch | Tag when finished |
|---|---|---|
| CP1: Technical Foundation | `cp1-technical-foundation` | `v1.0-cp1` |
| CP2: Transformation and Animation | `cp2-transformation-animation` | `v2.0-cp2` |
| CP3: OpenGL Integration and Prototype | `cp3-opengl-prototype` | `v3.0-cp3` |
| Final Project | `final-project` | `v4.0-final` |

**Task branch names:** `feature/cpN-short-task`, for example `feature/cp1-midpoint-circle`.
For bug fixes use `fix/cpN-short-bug`. Use lowercase letters and hyphens only.

---

## 3. Who Works on What

Each checkpoint belongs to one pair. The Project Lead creates the checkpoint branch when it is that pair's turn.

| Checkpoint | Pair | 
|---|---|
| CP1 | Pair A |
| CP2 | Pair B |
| CP3 | Pair C |
| Final | All three pairs (each pair has an area) |

**While another pair owns the checkpoint, you are not idle.** You review their Pull Requests when asked, and you prepare for your own turn (read the notes, try out the tools, write small test programs on a `docs/` or `feature/` branch).

Inside each pair, one person is **Partner 1** and the other is **Partner 2**. You review each other's work.

### CP1: Technical Foundation (Pair A)

Goal: use our own C++ drawing algorithms to build simple project components.

| Partner | Task branch | Task |
|---|---|---|
| Partner 1 | `feature/cp1-build-and-framebuffer` | CMake setup, pixel buffer, `setPixel`, save image to file. **Do this first.** |
| | `feature/cp1-midpoint-circle` | Midpoint circle algorithm |
| | `feature/cp1-bresenham-line` | Bresenham line algorithm |
| | `feature/cp1-polygon-fill` | Polygon fill |
| Partner 2 | `feature/cp1-components-shapes` | Draw Motor, Gear, Pulley, Lightbulb using the primitives |
| | `feature/cp1-belt-and-toolbar` | Draw the belt, toolbar and Play, Pause, Reset buttons |
| | `feature/cp1-picking` | Point-in-Circle click detection |
| | `feature/cp1-alpha-glow` | Alpha blending and glow around the bulb |
| Both | `docs/cp1-readme-tests` | Tests, README, screenshots |

### CP2: Transformation and Animation (Pair B)

| Partner | Task branch | Task |
|---|---|---|
| Partner 1 | `feature/cp2-transforms` | Translation and rotation (matrices) |
| | `feature/cp2-game-loop` | Animation loop and gear rotation |
| | `feature/cp2-drag-and-drop` | Drag parts onto the canvas, Play, Pause, Reset |
| Partner 2 | `feature/cp2-connections` | Connect parts with belts or meshing gears |
| | `feature/cp2-gear-ratios` | Speed passed from the motor through the chain |
| | `feature/cp2-darkness-light` | Darkness overlay that brightens with power |
| Both | `docs/cp2-readme-tests` | Tests, README, screenshots or demo |

### CP3: OpenGL Integration and Prototype (Pair C)

| Partner | Task branch | Task |
|---|---|---|
| Partner 1 | `feature/cp3-opengl-setup` | OpenGL window, input, CMake changes |
| | `feature/cp3-shapes-shaders` | Shapes and basic shaders |
| | `feature/cp3-ui-opengl` | Toolbar and buttons in OpenGL |
| Partner 2 | `feature/cp3-animation-opengl` | Rotation and movement in OpenGL |
| | `feature/cp3-simulation-link` | Connect the simulation to the OpenGL scene |
| | `feature/cp3-light-opengl` | Glow and darkness using OpenGL blending |
| Both | `docs/cp3-readme-tests` | Tests, README, prototype demo |

### Final Project (all pairs)

| Pair | Branch | Area |
|---|---|---|
| Pair A | `feature/final-ui-testing` | Interface polish, full testing, bug list |
| Pair B | `feature/final-simulation-animation` | Efficiency rules, edge cases, smooth animation |
| Pair C | `feature/final-graphics-lighting` | Visual polish, lighting, demo video |

---

## 4. One-Time Setup (everyone)

Check Git is installed:

```bash
git --version
```

Set your name and email (use your GitHub details):

```bash
git config --global user.name "Your Full Name"
git config --global user.email "your.email@example.com"
git config --global init.defaultBranch main
```

Copy the repository to your computer (the Project Lead will share the link):

```bash
git clone https://github.com/<owner-username>/lumina-gear.git
cd lumina-gear
```

---

## 5. Your Routine for Every Task

Replace `cp1-technical-foundation` and the task name with your own.

**Step 1: Get the latest checkpoint branch**

```bash
git fetch origin
git checkout cp1-technical-foundation
git pull origin cp1-technical-foundation
```

**Step 2: Create your task branch**

```bash
git checkout -b feature/cp1-midpoint-circle
```

**Step 3: Do your work. Check what changed**

```bash
git status
git diff
```

**Step 4: Save your work (commit)**

```bash
git add src/draw.cpp include/lumina/draw.h
git commit -m "feat: add midpoint circle algorithm"
```

Commit small and often.

**Step 5: Bring in your partner's latest work (at least once a day)**

```bash
git fetch origin
git merge origin/cp1-technical-foundation
```

If Git reports a conflict, go to [Section 7](#7-merge-conflicts).

**Step 6: Make sure it builds**

```bash
cd build
cmake ..
cmake --build .
cd ..
```

**Step 7: Upload your branch**

First time:

```bash
git push -u origin feature/cp1-midpoint-circle
```

After that:

```bash
git push
```

**Step 8: Open a Pull Request on GitHub**

1. Open the repository on GitHub and click **Compare & pull request**.
2. Set **base** to your checkpoint branch (for example `cp1-technical-foundation`). **Never `main`.**
3. Write one or two lines on what you did and how to test it.
4. Choose your **partner** as the reviewer.
5. Click **Create pull request**.

**Step 9: Partner reviews and merges**

Your partner checks that the code builds and works, clicks **Approve**, then **Squash and merge**. If something is wrong, they leave a comment and you fix it and push again.

**Step 10: Clean up**

```bash
git checkout cp1-technical-foundation
git pull origin cp1-technical-foundation
git branch -d feature/cp1-midpoint-circle
```

Start again from Step 1 for your next task.

---

## 6. Joining a Checkpoint Branch for the First Time

When the Project Lead announces a new checkpoint branch, run:

```bash
git fetch origin
git checkout cp2-transformation-animation
git pull origin cp2-transformation-animation
```

---

## 7. Merge Conflicts

A conflict means two people changed the same lines.

**Step 1:** See which files have conflicts:

```bash
git status
```

**Step 2:** Open the file. You will see:

```
<<<<<<< HEAD
your code
=======
the other person's code
>>>>>>> origin/cp1-technical-foundation
```

**Step 3:** Keep the correct code. Delete the three marker lines.

**Step 4:** Finish the merge:

```bash
git add src/draw.cpp
git commit -m "merge: resolve conflict in draw.cpp"
git push
```

To cancel and start over:

```bash
git merge --abort
```

**To avoid conflicts:** stay in your own files, pull often, and keep task branches short (a few days at most). Ask your partner before changing a file that is not yours.

---

## 8. Fixing Mistakes

| Situation | Command |
|---|---|
| Throw away changes in one file (not saved yet) | `git restore src/draw.cpp` |
| Unstage a file but keep the changes | `git restore --staged src/draw.cpp` |
| Change the last commit message (not pushed yet) | `git commit --amend -m "new message"` |
| Undo the last commit, keep the changes | `git reset --soft HEAD~1` |
| Undo a commit that is already pushed | `git revert <commit-hash>` |
| Hide unfinished work for a moment | `git stash` |
| Bring hidden work back | `git stash pop` |
| See recent history | `git log --oneline` |

**Never** use `git push --force` or `git reset --hard` on shared branches. If you are unsure, ask the Project Lead first.

---

## 9. Commit Messages

Format: `type: short description`

| Type | Meaning |
|---|---|
| `feat` | New feature |
| `fix` | Bug fix |
| `docs` | Documentation |
| `test` | Tests |
| `chore` | Setup, build files |
| `merge` | Conflict fix |

Good: `feat: add Bresenham line algorithm`
Bad: `update`, `fixed stuff`, `final`

---

## 10. Cheat Sheet

```bash
git status                          # what changed
git branch                          # list local branches
git checkout <branch>               # switch branch
git checkout -b <new-branch>        # create and switch
git add <file>                      # stage a file
git commit -m "feat: message"       # save a commit
git fetch origin                    # download updates
git pull origin <branch>            # download and merge
git merge origin/<branch>           # merge a remote branch into yours
git push                            # upload commits
git push -u origin <branch>         # upload a new branch (first time)
git branch -d <branch>              # delete a local branch
git log --oneline --graph --all     # see history as a graph
git stash / git stash pop           # hide / restore unfinished work
```

---

## 11. Team Rules

1. Never commit directly to `main` or a checkpoint branch. Use a task branch and a Pull Request.
2. Pull the latest checkpoint branch before you start each day.
3. Commit small and often, with clear messages.
4. Your code must build before you open a Pull Request.
5. Your partner reviews within 24 hours.
6. Do not commit build files (`build/`, `.o`, `.exe`, `.vscode/`, `.idea/`). The `.gitignore` covers this.
7. Never use `git push --force`.
8. Tell the Project Lead early if you are stuck or running late.
