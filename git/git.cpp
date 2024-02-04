#include <git/git.h>
#include <git/ui_git.h>
#include <git2.h>
#include <iostream>
#include <qdir.h>
#include <git/git_util.h>
#ifndef _WIN32
# include <unistd.h>
#endif
#include <errno.h>

int print_matched_cb(const char *path, const char *matched_pathspec, void *payload);
bool check_lg2(int error, const char *message, const char *extra);
static int readline(char **out);
static int ask(char **out, const char *prompt, char optional);
int cred_acquire_cb(git_credential **out, const char *url, const char *username_from_url,
                    unsigned int allowed_types,
                    void *payload);
bool match_arg(int *out, struct args_info *args, const char *opt);

Git::Git(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::Git)
{
    statusBar = new QStatusBar();

    git_libgit2_init();
    repoPath = QDir::currentPath() + "/Splitted-Media";
    init();
}

void Git::init()
{
    QDir repoDir(repoPath);
    if(!repoDir.exists()) {
        qDebug() << "Initialing" << Qt::endl;
        repoDir.mkpath(repoPath);
    }

    if (!doesRepositoryExist(repoPath.toUtf8())) {

        int error = git_repository_init(&repo, repoPath.toUtf8(), 0);
        if (error < 0) {
            const git_error* e = giterr_last();
            qCritical() << "Error initializing repository:" << error << e->message;
            return;
        }

        qDebug() << "Repository initialized at:" << repoPath;

    } else {
        qDebug() << "Repository already exists at:" << repoPath << Qt::endl;
        qDebug() << "Opening Repository\n";

        int error = git_repository_open(&repo, repoPath.toUtf8());
        if (error < 0) {
            const git_error* e = giterr_last();
            qCritical() << "Error opening repository:" << error << e->message;
            git_libgit2_shutdown();
            return;
        }
        qDebug() << "Opening Repository was successful\n";
    }


}

void Git::add()
{
    // git_index *index = nullptr;
    // int error = git_repository_index(&index, repo);
    // if (error < 0) {
    //     const git_error* e = giterr_last();
    //     qCritical() << "Error getting repository index:" << error << e->message;
    //     return;
    // }

    // error = git_index_add_all(index, nullptr, 0, nullptr, nullptr);
    // if (error < 0) {
    //     const git_error* e = giterr_last();
    //     qCritical() << "Error adding files to index:" << error << e->message;
    //     git_index_free(index);
    //     return;
    // }

    // git_index_write(index);
    // git_index_free(index);

    // qDebug() << "Files added to the index.";

    git_index_matched_path_cb matched_cb = NULL;
    git_index *index;
    git_strarray array = {0};
    struct index_options options = {0};
    options.mode = INDEX_ADD;
    options.verbose = 1;
    options.dry_run = 0;
    options.add_update = 0;
    options.repo = repo;

    matched_cb = print_matched_cb;
    /* Grab the repository's index. */
    check_lg2(git_repository_index(&index, repo), "Could not open repository index", NULL);
    git_index_add_all(index, nullptr, 0, matched_cb, &options);


}

void Git::commit()
{

    git_oid *treeId, commitId, *parentId;

    git_oid commit_oid,tree_oid;
    git_tree *tree;
    git_index *index;
    git_object *parent = nullptr;
    git_reference *ref = nullptr;
    git_signature *signature;

    int error = git_repository_index(&index, repo);
    if (error < 0) {
        const git_error* e = giterr_last();
        qCritical() << "Error getting repository index:" << error << e->message;
        return;
    }

    error = git_revparse_ext(&parent, &ref, repo, "HEAD");
    if (error == GIT_ENOTFOUND) {
        const git_error* err = giterr_last();
        qCritical() << "HEAD not found. Creating first commit: " << error << err->message;
    } else if (error != 0) {
        const git_error *err = git_error_last();
        if (err) qCritical() << "ERROR: " << err->klass << " Message: " << err->message;
        else qCritical() << "ERROR: " << error << "no detailed info\n";
    }
    if(check_lg2(git_repository_index(&index, repo), "Could not open repository index", nullptr)) {
        return;
    }
    if(check_lg2(git_index_write_tree(&tree_oid, index), "Could not write tree", nullptr)) {
        return;
    }

    if(check_lg2(git_index_write(index), "Could not write index", nullptr)) {
        return;
    }

    if(check_lg2(git_tree_lookup(&tree, repo, &tree_oid), "Error looking up tree", nullptr)) {
        return;
    }

    if(check_lg2(git_signature_default(&signature, repo), "Error creating signature", nullptr)) {
        return;
    }

    if(check_lg2(git_commit_create_v(
                  &commit_oid,
                  repo,
                  "HEAD",
                  signature,
                  signature,
                  NULL,
                  "comment",
                  tree,
                      parent ? 1 : 0, parent), "Error creating commit", NULL)) {
        return;
    }

    git_index_free(index);
    git_signature_free(signature);
    git_tree_free(tree);
    git_object_free(parent);
    git_reference_free(ref);

}

void Git::addRemoteUrl()
{
    const char* remoteName = "origin";
    const char* remoteUrl = "https://github.com/ljn7/testing-git-feature.git";;
    git_remote *remote = nullptr;

    // Check if the remote already exists
    int error = git_remote_lookup(&remote, repo, remoteName);

    if (error == 0) {
        // Remote exists, edit its URL
        error = git_remote_set_url(repo, remoteName, remoteUrl);

        if (error < 0) {
            const git_error* e = giterr_last();
            qCritical() << "Error setting remote URL:" << error << e->message;
        } else {
            qDebug() << "Remote '" << remoteName << "' URL updated successfully.";
        }

        git_remote_free(remote);
    } else {
        // Remote does not exist, create a new one
        error = git_remote_create(&remote, repo, remoteName, remoteUrl);

        if (error < 0) {
            const git_error* e = giterr_last();
            qCritical() << "Error adding remote:" << error << e->message;
        } else {
            qDebug() << "Remote '" << remoteName << "' added successfully with URL:" << remoteUrl;
        }
    }
}

void Git::push()
{
    git_push_options options;
    git_remote_callbacks callbacks;
    git_remote* remote = nullptr;
    char *refspec = const_cast<char*>("refs/heads/master");
    const git_strarray refspecs = {
        &refspec,
        1
    };

    check_lg2(git_remote_lookup(&remote, repo, "origin" ), "Unable to lookup remote", nullptr);

    check_lg2(git_remote_init_callbacks(&callbacks, GIT_REMOTE_CALLBACKS_VERSION),
              "Error initializing remote callbacks", nullptr);

    callbacks.credentials = cred_acquire_cb;

    check_lg2(git_push_options_init(&options, GIT_PUSH_OPTIONS_VERSION ), "Error initializing push", NULL);
    options.callbacks = callbacks;

    if(check_lg2(git_remote_push(remote, &refspecs, &options), "Error pushing", NULL)) {
        std::cerr << "Error Pushing";
        return;
    }

    std::cerr << "Pushed";

}

void Git::pull()
{

}

void Git::fetch()
{

}

void Git::refresh()
{

}

Git::~Git() {

    git_repository_free(repo);
    git_libgit2_shutdown();
}


bool Git::doesRepositoryExist(const char* path) {
    int error = git_repository_open(&repo, path);
    if (error == 0) {
        // Repository exists
        git_repository_free(repo);
        return true;
    }
    return false;
}

//callback fucntion for printing added files
int print_matched_cb(const char *path, const char *matched_pathspec, void *payload)
{
    struct index_options *opts = (struct index_options *)(payload);
    int ret;
    unsigned status;
    (void)matched_pathspec;

    /* Get the file status */
    if (git_status_file(&status, opts->repo, path) < 0)
        return -1;

    if ((status & GIT_STATUS_WT_MODIFIED) || (status & GIT_STATUS_WT_NEW)) {
        // printf("add '%s'\n", path);
        std::cerr << "add " << "'" << path << "'\n";
        ret = 0;
    } else {
        ret = 1;
    }

    if (opts->dry_run)
        ret = 1;

    return ret;
}

bool check_lg2(int error, const char *message, const char *extra)
{
    const git_error *lg2err;
    const char *lg2msg = "", *lg2spacer = "";

    if (!error)
        return false;

    if ((lg2err = git_error_last()) != NULL && lg2err->message != NULL) {
        lg2msg = lg2err->message;
        lg2spacer = " - ";
    }

    if (extra)
        qCritical() << message << " " << extra << " ["  << error << "] " <<  lg2spacer << " "  << lg2msg;
    else
        qCritical() << message << "[" << error << "]" << lg2spacer << lg2msg;

    return true;
}

static int readline(char **out)
{
    int c, error = 0, length = 0, allocated = 0;
    char *line = NULL;

    errno = 0;

    while ((c = getchar()) != EOF) {
        if (length == allocated) {
            allocated += 16;

            if ((line = (char*)realloc(line, allocated)) == nullptr) {
                error = -1;
                goto error;
            }
        }

        if (c == '\n')
            break;

        line[length++] = c;
    }

    if (errno != 0) {
        error = -1;
        goto error;
    }

    line[length] = '\0';
    *out = line;
    line = NULL;
    error = length;
error:
    free(line);
    return error;
}

static int ask(char **out, const char *prompt, char optional)
{
    printf("%s ", prompt);
    fflush(stdout);

    if (!readline(out) && !optional) {
        fprintf(stderr, "Could not read response: %s", strerror(errno));
        return -1;
    }
    return 0;
}

int cred_acquire_cb(git_credential **out,
                    const char *url,
                    const char *username_from_url,
                    unsigned int allowed_types,
                    void *payload)
{
    char *username = NULL, *password = NULL, *privkey = NULL, *pubkey = NULL;
    int error = 1;

    UNUSED(url);
    UNUSED(payload);

    if (username_from_url) {
        if ((username = strdup(username_from_url)) == NULL)
            goto out;
    } else if ((error = ask(&username, "Username:", 0)) < 0) {
        goto out;
    }

    if (allowed_types & GIT_CREDENTIAL_SSH_KEY) {
        int n;

        if ((error = ask(&privkey, "SSH Key:", 0)) < 0 ||
            (error = ask(&password, "Password:", 1)) < 0)
            goto out;

        if ((n = snprintf(NULL, 0, "%s.pub", privkey)) < 0 ||
            (pubkey = (char*)malloc(n + 1)) == NULL ||
            (n = snprintf(pubkey, n + 1, "%s.pub", privkey)) < 0)
            goto out;

        error = git_credential_ssh_key_new(out, username, pubkey, privkey, password);
    } else if (allowed_types & GIT_CREDENTIAL_USERPASS_PLAINTEXT) {
        if ((error = ask(&password, "Password:", 1)) < 0)
            goto out;

        error = git_credential_userpass_plaintext_new(out, username, password);
    } else if (allowed_types & GIT_CREDENTIAL_USERNAME) {
        error = git_credential_username_new(out, username);
    }

out:
    free(username);
    free(password);
    free(privkey);
    free(pubkey);
    return error;
}

bool match_arg(int *out, struct args_info *args, const char *opt)
{
    const char *found = args->argv[args->pos];

    if (!strcmp(found, opt)) {
        *out = 1;
        return 1;
    }

    if (!strncmp(found, "--no-", strlen("--no-")) &&
        !strcmp(found + strlen("--no-"), opt + 2)) {
        *out = 0;
        return 1;
    }

    *out = -1;
    return 0;
}
