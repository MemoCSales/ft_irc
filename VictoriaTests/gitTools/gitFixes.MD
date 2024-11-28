<!--
Use VScode viewer with ctrl+shift+v
-->
# Check git config
    ```sh
    git config --global --list
    ```
# Creating Sparse checkout
    ```sh
	#check if it is enable
    git config core.sparseCheckout
    ```

To achieve this, you can create a Git hook that will configure sparse-checkout for other users to exclude the submodule directory. However, Git hooks are not shared with the remote repository by default. They are local to each user's repository. You can include the hook script in your repository and instruct your team members to set it up locally.

### Step-by-Step Guide

1. **Create the main repository**: Initialize your main repository and add your files.

2. **Add the submodule**: Add the submodule to your repository.

3. **Create the hook script**: Create a script for the post-checkout hook and include it in your repository.

4. **Instruct team members to set up the hook**: Provide instructions for your team members to set up the hook locally.

### Detailed Steps

1. **Create the main repository**:

```sh
# Navigate to your desired directory
cd /path/to/your/main/repository

# Initialize the main repository
git init

# Add your files and commit
git add .
git commit -m "Initial commit"

# Add the remote repository
git remote add origin <remote-repository-url>
```

2. **Add the submodule**:

```sh
# Add the submodule
git submodule add <submodule-repository-url> submodule_directory

# Initialize the submodule
git submodule init

# Update the submodule
git submodule update

# Commit the changes
git add .
git commit -m "Add submodule"
```

3. **Create the hook script**:

Create a script for the post-checkout hook and include it in your repository. For example, create a file named `setup-hooks.sh`:

```sh
# Create the setup-hooks.sh script
touch setup-hooks.sh

# Make the script executable
chmod +x setup-hooks.sh

# Edit the script
nano setup-hooks.sh
```

Add the following content to `setup-hooks.sh`:

```sh
#!/bin/sh

# Create the post-checkout hook
cat << 'EOF' > .git/hooks/post-checkout
#!/bin/sh

# Check if the current user is the specific user
if [ "$(whoami)" != "your_username" ]; then
  # Enable sparse-checkout
  git config core.sparseCheckout true

  # Create the sparse-checkout file
  echo "/*" > .git/info/sparse-checkout
  echo "!submodule_directory" >> .git/info/sparse-checkout

  # Update the working directory to match the sparse-checkout file
  git read-tree -mu HEAD
fi
EOF

# Make the post-checkout hook executable
chmod +x .git/hooks/post-checkout
```

Replace `your_username` with your actual username and `submodule_directory` with the directory name of your submodule.

4. **Commit and push the hook script**:

```sh
# Add the setup-hooks.sh script to the repository
git add setup-hooks.sh
git commit -m "Add setup-hooks.sh script"
git push origin main
```

5. **Instruct team members to set up the hook**:

Provide instructions for your team members to set up the hook locally. For example, you can include the following instructions in your `README.md` or another documentation file:

```markdown
# Setting Up Git Hooks

To configure the repository to exclude the submodule directory, run the following script:

```sh
./setup-hooks.sh
```

# SET GIT HOOKS

This will set up the necessary Git hooks to configure sparse-checkout for your local repository.


### Explanation

1. **Create the main repository**: Initialize your main repository and add your files.

2. **Add the submodule**: Add the submodule to your repository and commit the changes.

3. **Create the hook script**: Create a script for the post-checkout hook and include it in your repository. This script will configure sparse-checkout for other users to exclude the submodule directory.

4. **Instruct team members to set up the hook**: Provide instructions for your team members to set up the hook locally. They need to run the `setup-hooks.sh` script to configure the post-checkout hook in their local repository.

By following these steps, you will have a submodule in the repository that only you will be able to pull from the main branch. Other users won't see the submodule in their local repository due to the post-checkout hook configuration.

# Handling team Merge issues
To control branches and avoid merge problems in a team project, follow these best practices:

1. **Use Feature Branches**: Create a new branch for each feature or bug fix. This keeps the main branch clean and stable.
    ```sh
    git checkout -b feature/your-feature-name
    ```

2. **Regularly Pull Changes**: Frequently pull changes from the main branch to keep your branch up to date and reduce merge conflicts.
    ```sh
    git pull origin main
    ```

3. **Commit Often**: Make small, frequent commits with clear messages. This makes it easier to track changes and resolve conflicts.
    ```sh
    git commit -m "Clear and concise commit message"
    ```

4. **Use Pull Requests**: Use pull requests (PRs) to review and discuss changes before merging them into the main branch. This ensures code quality and consistency.
    ```sh
    git push origin feature/your-feature-name
    # Then create a PR on your Git hosting service (e.g., GitHub, GitLab)
    ```

5. **Code Reviews**: Conduct code reviews for all PRs. This helps catch issues early and ensures that multiple team members are familiar with the codebase.

6. **Automated Testing**: Set up automated tests to run on each PR. This ensures that new changes do not break existing functionality.

7. **Rebase Instead of Merge**: When updating your feature branch with changes from the main branch, prefer rebasing to keep a clean commit history.
    ```sh
    git fetch origin
    git rebase origin/main
    ```

8. **Resolve Conflicts Early**: If conflicts arise, resolve them as soon as possible to avoid larger issues later.

9. **Communicate**: Keep open communication with your team about the changes you are making and any potential conflicts.

By following these practices, you can minimize merge problems and maintain a smooth workflow in your team project.

# Git Good Practices
## Team colaboration
To control branches and avoid merge problems in a team project, follow these best practices:

1. **Use Feature Branches**: Create a new branch for each feature or bug fix. This keeps the main branch clean and stable.
    ```sh
    git checkout -b feature/your-feature-name
    ```

2. **Regularly Pull Changes**: Frequently pull changes from the main branch to keep your branch up to date and reduce merge conflicts.
    ```sh
    git pull origin main
    ```

3. **Commit Often**: Make small, frequent commits with clear messages. This makes it easier to track changes and resolve conflicts.
    ```sh
    git commit -m "Clear and concise commit message"
    ```

4. **Use Pull Requests**: Use pull requests (PRs) to review and discuss changes before merging them into the main branch. This ensures code quality and consistency.
    ```sh
    git push origin feature/your-feature-name
    # Then create a PR on your Git hosting service (e.g., GitHub, GitLab)
    ```

5. **Code Reviews**: Conduct code reviews for all PRs. This helps catch issues early and ensures that multiple team members are familiar with the codebase.

6. **Automated Testing**: Set up automated tests to run on each PR. This ensures that new changes do not break existing functionality.

7. **Rebase Instead of Merge**: When updating your feature branch with changes from the main branch, prefer rebasing to keep a clean commit history.
    ```sh
    git fetch origin
    git rebase origin/main
    ```

8. **Resolve Conflicts Early**: If conflicts arise, resolve them as soon as possible to avoid larger issues later.

9. **Communicate**: Keep open communication with your team about the changes you are making and any potential conflicts.

By following these practices, you can minimize merge problems and maintain a smooth workflow in your team project.

# Track folders locally but ignore when merge or push are used
To keep some folders locally but ignore them when pushing to the remote repository or merging to the main branch, you can use the following approach:

1. Add the folders to your `.gitignore` file to ensure they are not tracked by Git.
2. Use the `git update-index --assume-unchanged` command to mark the files or folders as unchanged.

Here are the steps:

1. Add the folders to your `.gitignore` file:

```
.vscode/
VictoriaTests/
```

2. Mark the folders as unchanged:

```sh
git update-index --assume-unchanged .vscode/
git update-index --assume-unchanged VictoriaTests/
```

This will ensure that these folders are ignored by Git and not included in any pushes or merges. They will remain in your local repository but will not be tracked or pushed to the remote repository.

## Pulling a Non-Existent Remote Branch Locally

To pull a remote branch that does not exist locally, you can follow these steps:

1. **Fetch all branches from the remote**: This updates your local copy of the remote repository, including all branches.

   ```sh
   git fetch --all
   ```

2. **Checkout the branch**: This creates a new local branch that tracks the remote branch. Replace `branch-name` with the name of the remote branch you want to check out.
   ```sh
   git checkout -b branch-name origin/branch-name
   ```

This sequence of commands first ensures that your local repository is aware of all the branches in the remote repository, including the one you're interested in. Then, by using `git checkout -b`, you create a new local branch that is set up to track the corresponding remote branch.

## Check files in remote branch

```bash
git ls-tree -r --name-only $(git rev-parse origin/remote-branch-name)
```

## Pull files from another branch

```bash
git checkout origin/remote-branch-name -- path/to/file1 path/to/file2
```

# Creating new branches

To create a new branch from another branch in Git, you first need to switch to
the branch you want to base your new branch on using the `git checkout` command.
After that, you can create the new branch. Here's how you can do it:

```bash
git checkout existing-branch-name
git branch new-branch-name
git checkout new-branch-name
```

Alternatively, you can create and switch to the new branch in a single command
using the `-b` option with the `git checkout` command:

```bash
git checkout -b new-branch-name existing-branch-name
# git checkout -b cpp01ex03-003 $(git branch --show-current)

```

Replace `existing-branch-name` with the name of the branch you want to base your
new branch on, and `new-branch-name` with the name you want to give to your new
branch.

# Merge branches

## Switch to the branch you want to merge into (for example, master)

```bash
git checkout master
```

## Merge the other branch into your current branch (for example, branch-to-merge)

```bash
git merge branch-to-merge
```

## Change merge default message

```bash
git merge --e branch-to-merge
```

# Delete the merged branch locally

```bash
git branch -d branch-to-merge
```

# Delete the merged branch remotely

```bash
git push origin --delete branch-to-merge
```

# Edit merge message

To merge a branch and then edit the merge commit message. Here's how you can do it:

```bash
# Step 1: Merge the branch. Replace 'branch_name' with the name of your branch.
git merge branch_name --no-commit --no-ff

# Step 2: If there are no conflicts, you can commit with a custom message.
git commit -m "Your custom merge message"
```

Please replace `'branch_name'` with the name of the branch you want to merge and replace `'Your custom merge message'` with your desired commit message.

# Rename branch

To rename a branch in Git, you can use the following commands:

```bash
# Switch to the branch you want to rename
git checkout old-branch-name

# Rename the branch
git branch -m new-branch-name
```

If you want to rename the branch while you are on a different branch, you can do so with the following command:

```bash
git branch -m old-branch-name new-branch-name
```

## Delete and rename remote branch

1. Delete the old-name remote branch and push the new-name local branch:

```bash
git push origin :old-name new-name
```

Reset the upstream branch for the new-name local branch:

```bash
git push origin -u new-name
```

2. Another option is

```bash
# Delete the old branch on remote
git push origin --delete old-branch-name

# Push the new branch to remote
git push origin new-branch-name
```

# Commit : Amend previous message

The `git commit --amend` command is used to modify the most recent commit. It doesn't work for commits that are not the latest. If you want to modify a commit that is not the latest, you can use an interactive rebase.

<!--git commit --amend --no-edit -->

```bash
git push origin --force-with-lease $(git branch --show-current)
```

Here's how you can do it:

```bash
git rebase -i HEAD~3
```

This command will open a text editor with a list of the last 3 commits. Each commit is represented by a line that starts with the word `pick`.

To modify a commit, replace `pick` with `edit` for that commit. Then save and close the editor.

Git will now pause the rebase at the commit you want to edit. You can amend the commit as you want. For example, you can modify the files or use `git commit --amend` to change the commit message.

Once you're done, you can continue the rebase with:

```bash
git rebase --continue
```

Please note that this will rewrite the commit history, which can cause problems for others if they have cloned or pulled your repository. Be careful when using `git rebase`.

# Add commit template to local repository

```bash
git config --local commit.template {path-to-commit-template}
```

# Adding SUBMODULES:

GitHub Copilot: To add a submodule to a Git project, you can use the `git submodule add` command. Here's how you can do it:

1. Navigate to your project directory in the terminal.

```bash
cd /path/to/your/project
```

2. Add the submodule. Replace `https://github.com/username/repo.git` with the URL of the repository you want to add as a submodule.

```bash
git submodule add https://github.com/username/repo.git
```

By default, the submodule will be added in a directory named after the repository. If you want to add it in a different directory, you can specify the path as a second argument:

```bash
git submodule add https://github.com/username/repo.git path/to/submodule
```

After adding the submodule, you can directly initialize and update it (if necessary) with:

```sh
git submodule update --init --recursive
```

However, to add and initialize in one go, you just use the `git submodule add` command, and initialization is implicitly done when you clone the repository with the `--recurse-submodules` option or when other users clone your repository.

3. **Commit the changes** to save the addition of the submodule to your project:

```sh
git add .gitmodules <path-to-submodule>
git commit -m "Added and initialized submodule at <path-to-submodule>"
```

4. **Push the changes** to your remote repository (if applicable):

```sh
git push
```

5. UpdateSubmodule

```bash
git submodule sync
git submodule update --init --recursive
```

Now, the submodule is added to your project. When you clone the project, use the `--recursive` option to also clone all submodules:

```bash
git clone --recursive https://github.com/yourusername/yourproject.git
```

If you've already cloned the project and want to fetch the submodules, you can use the `git submodule update` command:

```bash
git submodule update --init --recursive
```

# Fetch specific files from remote branch

To restore a specific folder from a remote branch in Git, you can use a combination of `fetch` and `checkout`. Here's how you can do it:

1. First, fetch the remote branch:

```bash
git fetch origin <branch-name>
```

2. Then, checkout the specific folder you want to restore:

```bash
git checkout origin/<branch-name> -- <folder-path>
```

This will update the specified folder in your current branch with the version from the remote branch.

Please note that this operation will overwrite the specified folder in your current branch. If you have any local changes in this folder that you want to keep, you should commit or stash them before running these commands.

# delete submodule

To delete a Git submodule, you need to:

1. Delete the relevant section from the `.gitmodules` file. The section would look something like this:

```ini
[submodule "SubmoduleName"]
    path = SubmodulePath
    url = git://github.com/username/repo.git
```

2. Stage the `.gitmodules` changes via Git:

```bash
git add .gitmodules
```

3. Delete the relevant section from `.git/config`, which looks like the section in `.gitmodules`.

4. Run `git rm --cached path_to_submodule` (no trailing slash).

```bash
git rm --cached path_to_submodule
```

5. Run `rm -rf .git/modules/path_to_submodule` (no trailing slash).

```bash
rm -rf .git/modules/path_to_submodule
```

6. Commit the changes:

```bash
git commit -m "Removed submodule "
```

7. Delete the now untracked submodule files:

```bash
rm -rf path_to_submodule
```

Remember to replace `SubmoduleName`, `SubmodulePath`, and `path_to_submodule` with the name and path of your specific submodule.

# change Submodule path

To change the path of a submodule in Git, you need to:

1. **Open the `.gitmodules`** file in the root of your repository.

2. **Find the section** for the submodule you want to change. It will look something like this:

```ini
[submodule "SubmoduleName"]
    path = old/path/to/submodule
    url = git://github.com/username/repo.git
```

3. **Change the `path`** to the new path you want:

```ini
[submodule "SubmoduleName"]
    path = new/path/to/submodule
    url = git://github.com/username/repo.git
```

4. **Stage** the `.gitmodules` changes:

```bash
git add .gitmodules
```

5. Now, you need to **reflect this change** in the Git index:

```bash
git mv old/path/to/submodule new/path/to/submodule
```

6. **Commit** the changes:

```bash
git commit -m "Moved submodule"
```

7. **Change** .git/modules/path/to/submodule

Remember to replace `SubmoduleName`, `old/path/to/submodule`, and `new/path/to/submodule` with the name and paths of your specific submodule.

# Divergent branches

<!--
 Safest option:
	hint:   git config pull.rebase false  # merge (the default strategy)
#hint:   git config pull.rebase true   # rebase
#hint:   git config pull.ff only       # fast-forward only
-->

The `git pull` command options `--rebase`, `--no-rebase`, and `--ff-only` affect
how Git handles divergent branches:

1. **`git pull --rebase`**: Replays your local commits on top of remote changes.
   This creates a linear commit history but modifies the existing history.
   (\*NOTE: Primarily use when changes are in local repo)
2. **`git pull --no-rebase`**: Merges remote changes into your local branch,
   creating a new merge commit. This maintains the exact commit history but can
   lead to a non-linear history.

3. **`git pull --ff-only`**: Only allows fast-forward merges. Git moves the branch
   pointer forward to catch up with the remote branch. If a fast-forward merge
   is not possible, the pull operation fails. This maintains a linear commit
   history and doesn't modify the existing history, but doesn't allow divergence
   between local and remote branches.
