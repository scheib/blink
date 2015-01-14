// Sends a message when the notificationclick event gets received.
var messagePort = null;

addEventListener('message', function(event) {
    messagePort = event.data;
    messagePort.postMessage('ready');
});

addEventListener('notificationclick', function(event) {
    messagePort.postMessage('Clicked on Notification: ' + event.notification.title);
});
