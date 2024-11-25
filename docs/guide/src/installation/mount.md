# Mount Setting (option)

In many experimental setups, tasks are often distributed across multiple servers, such as:

- **DAQ Server**: Handles the DAQ process.
- **File Server**: Stores experimental data.
- **Analysis Server**: Performs data analysis.

To simplify workflows, **NFS (Network File System)** can be used to allow the analysis server to access data directly from the file server without duplicating files.
Additionally, external storage devices can be mounted for offline analysis to store data or generated ROOT files.

---

## Configuring the File Server for NFS

### Step 1: Install NFS Server Utilities

```bash
sudo apt update
sudo apt install nfs-kernel-server
```

### Step 2: Configure Shared Directories in `/etc/exports`

1. Edit the `/etc/exports` file:

   ```bash
   sudo vi /etc/exports
   ```

2. Add an entry for the directory to share:

   ```plaintext
   /path/to/shared/data <client_ip>(ro,sync,subtree_check)
   ```

   - Replace `/path/to/shared/data` with the directory you want to share.
   - Replace `<client_ip>` with the IP address or subnet (e.g., `192.168.1.*`).

3. Common options in `/etc/exports`
   | Option | Description |
   |-----------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------|
   | **`rw`** | Allows read and write access. |
   | **`ro`** (default) | Allows read-only access. |
   | **`sync`** (default) | Commits changes to disk before notifying the client. This ensures data integrity but may slightly reduce speed. |
   | **`async`** | Allows the server to reply to requests before changes are committed to disk. This improves speed but risks data corruption in case of failure. |
   | **`subtree_check`** (default) | Ensures proper permissions for subdirectories but may reduce performance. |
   | **`no_subtree_check`**| Disables subtree checks for better performance but reduces strict access control. |
   | **`wdelay`** (default) | Delays disk writes to combine operations for better performance. Improves performance but increases the risk of data loss during failures. |
   | **`no_wdelay`** | Disables delays for immediate write operations, reducing risk of data loss but potentially decreasing performance. |
   | **`hide`** | Prevents overlapping mounts from being visible to clients. Enhances security by hiding overlapping mounts. |
   | **`nohide`** | Allows visibility of overlapping mounts. Useful for nested exports but can lead to confusion. |
   | **`root_squash`** (default) | Maps the root user of the client to a non-privileged user on the server. Improves security by preventing root-level changes. |
   | **`no_root_squash`** | Allows the root user of the client to have root-level access on the server. This is not recommended unless absolutely necessary. |
   | **`all_squash`** | Maps all client users to a single anonymous user on the server. Useful for shared directories with limited permissions. |

4. Save and exit the editor.

### Step 3: Apply Changes and Start NFS Server

```bash
sudo exportfs -a
sudo systemctl enable nfs-server
sudo systemctl start nfs-server
```

---

## Configuring the Analysis Server for Mounting

### 1. Mounting a Shared Directory via NFS

#### Step 1: Install NFS Utilities:

```bash
sudo apt update
sudo apt install nfs-common
```

#### Step 2: Create a Mount Point:

```bash
sudo mkdir -p /mnt/data
```

#### Step 3: Configure Persistent Mounting:

```bash
sudo vi /etc/fstab
```

Add:

```plaintext
<file_server_ip>:/path/to/shared/data /mnt/data nfs defaults 0 0
```

#### Step 4: Apply and Verify:

```bash
sudo mount -a
df -h
```

### 2. Mounting External Storage (e.g., USB or HDD)

#### Step 1: Identify the Device:

```bash
lsblk
```

- Look for the device name (e.g., `/dev/sdb1`) in the output.

#### Step 2: Create a Mount Point:

```bash
sudo mkdir -p /mnt/external
```

#### Step 3: Configure Persistent Mounting:

```bash
sudo vi /etc/fstab
```

Add:

```plaintext
/dev/sdb1 /mnt/external ext4 defaults 0 0
```

- Replace `/dev/sdb1` with the actual device name.
- Replace `   ext4` with the correct filesystem type (e.g., `ext4`, `xfs`, `vfat`).

#### Step 4: Apply and Verify:

```bash
sudo mount -a
df -h
```

---

## Troubleshooting

- **File Server Issues**:

  - Ensure the NFS service is running on the file server:

  ```bash
  sudo systemctl status nfs-server
  ```

  - Verify the export list:

  ```bash
  showmount -e
  ```

- **Analysis Server Issues**:

  - Check the NFS mount status:

  ```bash
  sudo mount -v /mnt/data
  ```

  - Verify network connectivity between the analysis server and file server.

- **External Storage Issues**:

  - Unmount safety:

  ```bash
  sudo umount /mnt/external
  ```

  - Formatting uninitialized Storage:

  ```bash
  sudo mkfs.ext4 /dev/sdb1
  ```

  - Use UUIDs for reliable mounting to avoid issues with device naming (e.g., `/dev/sdb1`):

  ```bash
  sudo blkid /dev/sdb1
  ```

  Add to `/etc/fstab`:

  ```plaintext
  UUID=your-uuid-here /mnt/external ext4 defaults 0 0
  ```

## Example Configuration

### File Server (`/etc/exports`)

```plaintext
/data/shared 192.168.1.101(rw,sync,no_subtree_check)
/data/backup 192.168.1.102(ro,async,hide)
```

### Analysis Server (`/etc/fstab`)

```plaintext
192.168.1.100:/data/shared /mnt/data nfs defaults 0 0
UUID=abc123-4567-89def /mnt/external ext4 defaults 0 0
```
