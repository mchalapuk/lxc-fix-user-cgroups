#include <iostream>
#include <functional>
#include <memory>
#include <fstream>

#include <sys/types.h>
#include <signal.h>
#include <pwd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

void err(std::string const& message, std::string const& arg = "") {
  std::cerr << message;
  if (arg != "") std::cerr << " " << arg;
  std::cerr << std::endl;
  exit(1);
}

void for_each_file_in_folder(
    std::string folder,
    std::function<void(std::string, std::string)> func
    ) {

  DIR *dir = opendir(folder.c_str());
  if (!dir) {
    err("no such file or directory:", folder);
  }
  auto deleter = [] (DIR *dir) { closedir(dir); };
  std::unique_ptr<DIR, decltype(deleter)> scope_guard(dir, deleter);

  struct dirent *ent;
  while ((ent = readdir(dir)) != nullptr) {
    if (ent->d_name[0] == '.') {
      continue;
    }
    func(folder + ent->d_name, ent->d_name);
  }
}

void create_folder(std::string folder, int id) {
  struct stat sb;
  if (stat(folder.c_str(), &sb) != 0) {
    if (mkdir(folder.c_str(), S_IRWXU) == -1) {
      perror("mkdir()");
      err("filed to create directory:", folder);
    }
    chown(folder.c_str(), id, id);
  }
}

#define INVALID_UID -1;

uid_t uid_of(char const* name) {
  struct passwd *pwd = getpwnam(name);
  if (pwd) {
    return pwd->pw_uid;
  } else {
    return INVALID_UID;
  }
}

void write_file(std::string file, std::string content) {
  std::fstream out(file);
  out << content << std::endl;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    err("usage: lxc-fix-user-cgroups <user> <parent-pid>");
  }
  char const* user = argv[1];
  char const* parent_pid = argv[2];

  if (getpwnam(user) == nullptr) {
    err("no such user:", user);
  } 
  if (kill(atoi(parent_pid), 0) == ESRCH) {
    err("process does not exist:", argv[2]);
  }

  int uid = uid_of(user);
  if (uid == -1) {
    err("failed to get uid of", user);
  }

  for_each_file_in_folder("/sys/fs/cgroup/",
      [user, uid, parent_pid] (std::string dir, std::string basename) {

        if (basename == "cpuset") {
          write_file(dir + "/cgroup.clone_children", "1");
        } else if (basename == "memory") {
          write_file(dir + "/memory.use_hierarchy", "1");
        }

        std::string user_dir(dir + "/" + user);
        create_folder(user_dir, uid);
        write_file(user_dir + "/tasks", parent_pid);
      });
}

