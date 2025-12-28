# December 2023 - Week 5
# Channel: #application-security
# Messages: 17

[2023-12-27 09:32] .mydev: Can someone help with these 2 challenges?
[Attachments: 983e0648-e671-4997-b1b7-dbeb50cd1015.jpg, 7ea829eb-bf6c-4d13-8e27-2f44900acde0.jpg, push-the-button.tar.gz, ace-win-king-cry.tar.gz]

[2023-12-27 09:44] naci: [replying to .mydev: "Can someone help with these 2 challenges?"]
is this ctf still running

[2023-12-27 09:44] .mydev: [replying to naci: "is this ctf still running"]
No. Just a practice for me

[2023-12-27 09:44] sariaki: what's it called

[2023-12-27 09:45] naci: should be https://ctf.stm.com.tr/ though it doesnt say anything about the date lol

[2023-12-27 09:45] naci: nvm

[2023-12-27 09:45] naci: october 27-28

[2023-12-27 09:47] naci: [replying to .mydev: "Can someone help with these 2 challenges?"]
for push-the-button, it wants you to push the button 15 times, but you are only allowed to push it 10 times, so you should investigate if you can bypass the allowed limit, you should look into the callback function under index.js line 38
```js
app.put('/click', (req, res) => {
```

[2023-12-27 09:49] .mydev: [replying to naci: "for push-the-button, it wants you to push the butt..."]
Ah. So I only have to change the code in this function via client side Inspect, to allow me 15 times?

[2023-12-27 09:49] naci: no, you are not allowed to change the code

[2023-12-27 09:50] naci: the files are provided for you so you can understand the source and exploit the vuln at the portal they provide you (website in this case)

[2023-12-27 09:51] naci: you find the vulnerability in the source files they provide, then you visit the link and exploit that vulnerability

[2023-12-27 09:52] naci: try to understand the conditions that allows you to add points to yourself

[2023-12-27 09:58] .mydev: [replying to naci: "try to understand the conditions that allows you t..."]
```Javascript
app.put('/click', (req, res) => {
if (req.session.clicks > 0) {
req.session.points++;
req.session.save();
```
As I understand it, it sends to the web server a cookie with clicks key that equals to the current amount of clicks in total
So I have to change it in client-side, such that it won't increment my clicks?

[2023-12-27 09:58] naci: you can try

[2023-12-27 10:02] .mydev: [replying to naci: "you can try"]
I'm on mobile mwanwhile... Will be on the PC later on... Thank you for that man.
What about the second one?

[2023-12-27 10:06] naci: not sure, i gtg, ill check again when i come back